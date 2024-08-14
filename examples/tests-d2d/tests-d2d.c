#include <stddef.h>
#include "twr-draw2d.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

unsigned long crc32(char* str, unsigned long len) {
   const long table_len = 256;
   unsigned long crc_table[table_len];
   unsigned long c;
   for (unsigned long n = 0; n < table_len; n++) {
      c = n;
      for (unsigned long k = 0; k < 8; k++) {
         c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
      }
      crc_table[n] = c;
   }

   unsigned long crc = (unsigned long)((long)0 ^ (-1));
   
   for (char* str_pos = str; str_pos < str+len; str_pos++) {
      crc = (crc >> 8) ^ crc_table[(crc ^ *str_pos) & 0xFF];
   }

   return (unsigned long)((long)crc ^ (-1)) >> 0;
}

unsigned short fletcher16(char* str) {
   unsigned short sum1 = 0xFF;
   unsigned short sum2 = 0xFF;

   for (char* str_ptr = str; *str_ptr != '\0'; str_ptr++) {
      sum1 = (sum1 + *str_ptr) % 255;
      sum2 = (sum2 + sum1) % 255;
   }

   return (sum2 << 8) | sum1;
}

void to_hex_string(unsigned long num, char* hex) {
   for (unsigned long i = 0; i < 8; i++) {
      int seg = (int)((num >> i*4) & 0xF);
      hex[7-i] = (char)(seg <= 9 ? '0'+seg : 'A'+seg-10);
   }
   hex[8] = '\0';
}



void test_hash(char* test, unsigned long expected_hash, unsigned long calc_hash) {
   if (calc_hash == expected_hash) {
      printf("%s test was successful!\n", test);
   } else {
      char canvas_hex[9];
      to_hex_string(calc_hash, canvas_hex);
      char expected_hex[9];
      to_hex_string(expected_hash, expected_hex);
      printf("%s test failed! Expected 0x%s got 0x%s\n", test, expected_hex, canvas_hex);
   }
}
#define width 600
#define height 600
#define mem_size width*height*4
char mem[mem_size];
void test_img_hash(struct d2d_draw_seq* ds, char* test, unsigned long hash) {

   d2d_getimagedata(ds, 1, 0, 0, width, height);
   d2d_imagedatatoc(ds, 1, (void*)mem, mem_size);
   d2d_releaseid(ds, 1);
   unsigned long canvas_hash = crc32(mem, mem_size);

   test_hash(test, hash, canvas_hash);
}


enum Test {
   EmptyCanvas,
   FillRect,
   Reset,
   StrokeRect,
   FillText,
   FillCodePoint,
   StrokeText,

   TextMetrics,
   SetLineWidth,
   SaveAndRestore,

   SetStrokeStyleRGBA,
   SetFillStyleRGBA,
   SetStrokeStyle,
   SetFillStyle,
   SetFont,
   SetLineCap,
   SetLineJoin,
   SetLineDash,
   GetLineDashLength,
   GetLineDash,
   SetLineDashOffset,

   BeginPathAndRectAndFill,
   RectAndStroke,
   MoveToAndLineTo,
   Arc,
   ArcTo,
   BezierTo,
   RoundRect,
   Ellipse,
   QuardraticCurveTo,
   ClosePath,

   ClearRect,
   Scale,
   Translate,
   Rotate,
   GetTransform,
   SetTransform,
   Transform,
   ResetTransform,

   CreateLinearGradient, //also tests releaseid, setfillstylegradient, and addColorStop
   CreateRadialGradient,

   ImageDataAndPutImageData,
   LoadAndDrawImage,

};

void test_case(int id) {
   struct d2d_draw_seq* ds = d2d_start_draw_sequence(1000);
   d2d_reset(ds);

   switch (id) {
      case EmptyCanvas:
      {
         test_img_hash(ds, "EmptyCanvas", 0xEBF5A8C4);
      }
      break;
      
      case FillRect:
      {
         d2d_fillrect(ds, 10.0, 10.0, 300.0, 300.0);
         test_img_hash(ds, "FillRect", 0xDF03FF9D);
      }
      break;

      case Reset:
      {
         //just uses the reset called above
         test_img_hash(ds, "Reset", 0xEBF5A8C4);
      }
      break;

      case StrokeRect:
      {
         d2d_strokerect(ds, 10.0, 10.0, 300.0, 300.0);
         test_img_hash(ds, "StrokeRect", 0xECBD3A0D);
      }
      break;

      case FillText:
      {
         d2d_filltext(ds, "Test Text", 50.0, 50.0);
         test_img_hash(ds, "FillText", 0x80C61DB8);
      }
      break;

      case FillCodePoint:
      {
         d2d_fillcodepoint(ds, 65, 50.0, 50.0);
         test_img_hash(ds, "FillCodePoint", 0x1A115D0E);
      }
      break;

      case StrokeText:
      {
         d2d_stroketext(ds, "Test Text", 50.0, 50.0);
         test_img_hash(ds, "StrokeText", 0x58A2C31A);
      }
      break;

      case TextMetrics:
      {
         struct d2d_text_metrics metrics;
         d2d_measuretext(ds, "Test Text", &metrics);
         test_hash("TextMetrics", 0x4E05379E, crc32((char*)&metrics, sizeof(struct d2d_text_metrics)));
      }
      break;

      case SetLineWidth:
      {
         d2d_setlinewidth(ds, 10.0);
         d2d_strokerect(ds, 50.0, 50.0, 500.0, 500.0);
         test_img_hash(ds, "SetLineWidth", 0x2609FF7F);
      }
      break;

      case SaveAndRestore:
      {
         d2d_setlinewidth(ds, 50.0);
         d2d_save(ds);
         d2d_setlinewidth(ds, 10.0);
         d2d_restore(ds);
         d2d_strokerect(ds, 50.0, 50.0, 500.0, 500.0);
         test_img_hash(ds, "SaveAndRestore", 0x256026AB);
      }
      break;

      case SetStrokeStyleRGBA:
      {
         d2d_setstrokestylergba(ds, 0xFF00FF30);
         d2d_strokerect(ds, 50.0, 50.0, 500.0, 500.0);
         test_img_hash(ds, "SetStrokeStyleRGBA", 0x59881A79);
      }
      break;

      case SetFillStyleRGBA:
      {
         d2d_setfillstylergba(ds, 0xFF00FF20);
         d2d_fillrect(ds, 50.0, 50.0, 500.0, 500.0);
         test_img_hash(ds, "SetFillStyleRGBA", 0xA80B06C2);
      }
      break;

      case SetStrokeStyle:
      {
         d2d_setstrokestyle(ds, "green");
         d2d_strokerect(ds, 50.0, 50.0, 500.0, 500.0);
         test_img_hash(ds, "SetStrokeStyle", 0x3FCD6506);
      }
      break;

      case SetFillStyle:
      {
         d2d_setfillstyle(ds, "green");
         d2d_fillrect(ds, 50.0, 50.0, 500.0, 500.0);
         test_img_hash(ds, "SetFillStyle", 0x6376FAF2);
      }
      break;

      case SetFont:
      {
         d2d_setfont(ds, "48px serif");
         d2d_filltext(ds, "Test Text", 50.0, 50.0);
         test_img_hash(ds, "SetFont", 0xA04DF203);
      }
      break;

      case SetLineCap:
      {
         d2d_setlinecap(ds, "round");
         d2d_setlinewidth(ds, 50.0);
         d2d_beginpath(ds);
         d2d_moveto(ds, 50.0, 50.0);
         d2d_lineto(ds, 100.0, 100.0);
         d2d_stroke(ds);
         test_img_hash(ds, "SetLineCap", 0x2F86CAF2);
      }
      break;

      case SetLineJoin:
      {
         d2d_setlinejoin(ds, "round");
         d2d_setlinewidth(ds, 25.0);
         d2d_beginpath(ds);
         d2d_moveto(ds, 50.0, 50.0);
         d2d_lineto(ds, 100.0, 100.0);
         d2d_lineto(ds, 150.0, 50.0);
         d2d_stroke(ds);
         test_img_hash(ds, "SetLineJoin", 0xDEB22C13);
      }
      break;

      case SetLineDash:
      {
         const double segments[6] = {10, 20, 15, 30, 20, 40};
         d2d_setlinedash(ds, 6, segments);
         d2d_moveto(ds, 50.0, 50.0);
         d2d_lineto(ds, 400.0, 400.0);
         d2d_stroke(ds);
         test_img_hash(ds, "SetLineDash", 0x58E1E91B);
      }
      break;

      case GetLineDashLength:
      {
         #define seg_len 6
         const double segments[seg_len] = {10, 20, 15, 30, 20, 40};
         d2d_setlinedash(ds, seg_len, segments);
         long len = d2d_getlinedashlength(ds);
         if (len == seg_len) {
            printf("GetLineDashLength test was successful!\n");
         } else {
            printf("GetLineDashLength test failed! Expected %ld got %ld\n", (long)seg_len, len);
         }
      }
      break;

      case GetLineDash:
      {
         #define seg_len 6
         const double segments[seg_len+1] = {10, 20, 15, 30, 20, 40, -1.0};
         d2d_setlinedash(ds, seg_len, segments);
         
         double seg_buffer[seg_len+1];
         seg_buffer[seg_len] = -1.0; //extra segment ensures that the lengths match too
         d2d_getlinedash(ds, seg_len+1, seg_buffer);

         bool correct = true;
         for (int i = 0; i < seg_len+1; i++) {
            if (segments[i] != seg_buffer[i]) {
               correct = false;
            }
         }
         if (correct) {
            printf("GetLineDash test was successful!\n");
         } else {
            printf("GetLineDash test failed! Expected {");
            for (int i = 0; i < seg_len+1; i++) {
               if (i != 0) printf(", ");
               printf("%f", segments[i]);
            }
            printf("} got {");
            for (int i = 0; i < seg_len+1; i++) {
               if (i != 0) printf(", ");
               printf("%f", seg_buffer[i]);
            }
            printf("}\n");
         }
      }
      break;

      case SetLineDashOffset:
      {
         const double segments[2] = {20.0, 20.0};
         d2d_setlinedash(ds, 2, segments);
         d2d_setlinedashoffset(ds, 10);
         d2d_beginpath(ds);
         d2d_moveto(ds, 0.0, 0.0);
         d2d_lineto(ds, width, height);
         d2d_stroke(ds);
         test_img_hash(ds, "SetLineDashOffset", 0x5E654AEA);
      }
      break;

      case BeginPathAndRectAndFill:
      { 
         d2d_beginpath(ds);
         d2d_rect(ds, 50.0, 50.0, 500.0, 500.0);
         d2d_fill(ds);
         test_img_hash(ds, "BeginPathAndRectAndFill", 0x259D8A55);
      }
      break;

      case RectAndStroke:
      {
         d2d_beginpath(ds);
         d2d_rect(ds, 50.0, 50.0, 500.0, 500.0);
         d2d_stroke(ds);
         test_img_hash(ds, "RectAndStroke", 0xD617CFBE);
      }
      break;

      case MoveToAndLineTo:
      {
         d2d_beginpath(ds);
         d2d_moveto(ds, 50.0, 50.0);
         d2d_lineto(ds, 300.0, 50.0);
         d2d_stroke(ds);
         test_img_hash(ds, "MoveToAndLineTo", 0x39277955);
      }
      break;

      case Arc:
      {
         d2d_beginpath(ds);
         d2d_arc(ds, 50.0, 50.0, 40.0, 0.0, 3.1415*1.5, false);
         d2d_stroke(ds);
         test_img_hash(ds, "Arc", 0x3B521B43);
      }
      break;

      case ArcTo:
      {
         d2d_beginpath(ds);
         d2d_moveto(ds, 0.0, 0.0);
         d2d_arcto(ds, width/2.0, 250.0, width, 0.0, 400.0);
         d2d_stroke(ds);
         test_img_hash(ds, "ArcTo", 0x5039DA68);
      }
      break;

      case BezierTo:
      {
         d2d_beginpath(ds);
         d2d_moveto(ds, 0.0, 0.0);
         d2d_bezierto(ds, 50.0, 100.0, width-75, 250.0, width, 0.0);
         d2d_stroke(ds);
         test_img_hash(ds, "BezierTo", 0x7FD136E1);
      }
      break;

      case RoundRect:
      {
         d2d_beginpath(ds);
         d2d_roundrect(ds, 50.0, 50.0, 500.0, 500.0, 30.0);
         d2d_fill(ds);
         test_img_hash(ds, "RoundRect", 0x88068679);
      }
      break;

      case Ellipse:
      {
         d2d_beginpath(ds);
         d2d_ellipse(ds, 100.0, 100.0, 20.0, 40.0, 3.1415*0.25, 0, 3.1415*2.0, false);
         d2d_fill(ds);
         test_img_hash(ds, "Ellipse", 0x41492067);
      }
      break;
      
      case QuardraticCurveTo:
      {
         d2d_beginpath(ds);
         d2d_moveto(ds, 0.0, 0.0);
         d2d_quadraticcurveto(ds, 250.0, 250.0, width, 0.0);
         d2d_stroke(ds);
         test_img_hash(ds, "QuardraticCurveTo", 0x89C55B66);
      }
      break;

      case ClosePath:
      {
         d2d_beginpath(ds);
         d2d_moveto(ds, 75.0, 50.0);
         d2d_lineto(ds, 100.0, 100.0);
         d2d_lineto(ds, 50.0, 100.0);
         d2d_closepath(ds);
         d2d_fill(ds);
         test_img_hash(ds, "ClosePath", 0x7243A8E2);
      }
      break;

      case ClearRect:
      {
         d2d_fillrect(ds, 50.0, 50.0, 500.0, 500.0);
         d2d_clearrect(ds, 100.0, 100.0, 400.0, 400.0);
         test_img_hash(ds, "ClearRect", 0xA4E118B3);
      }
      break;

      case Scale:
      {
         d2d_scale(ds, 10.0, 15.0);
         d2d_fillrect(ds, 10.0, 10.0, 30.0, 30.0);
         test_img_hash(ds, "Scale", 0x54E5E8DB);
      }
      break;

      case Translate:
      {
         d2d_translate(ds, 50.0, 50.0);
         d2d_fillrect(ds, 0.0, 0.0, 500.0, 500.0);
         test_img_hash(ds, "Translate", 0x259D8A55);
      }
      break;

      case Rotate:
      {
         d2d_rotate(ds, 50.0/180.0 * 3.1415);
         d2d_fillrect(ds, 250.0, 25.0, 50.0, 50.0);
         test_img_hash(ds, "Rotate", 0xB3C72BD5);
      }
      break;

      case GetTransform:
      {
         #define transform_len 3
         struct d2d_2d_matrix transforms[transform_len];
         
         d2d_gettransform(ds, &transforms[0]);
         
         d2d_translate(ds, 10.0, 500.0);
         d2d_gettransform(ds, &transforms[1]);

         d2d_rotate(ds, 3.1415 * 1.5);
         d2d_gettransform(ds, &transforms[2]);

         test_hash("GetTransform", 0x1996F72C, crc32((char*)transforms, sizeof(struct d2d_2d_matrix)*transform_len));
      }
      break;

      case SetTransform:
      {
         d2d_settransform(ds, 2.5, 10.0, 10.0, 2.0, 50.0, 50.0);
         d2d_fillrect(ds, 0.0, 0.0, 50.0, 50.0);
         test_img_hash(ds, "SetTransform", 0x0526828F);
      }
      break;

      case Transform:
      {
         d2d_transform(ds, 2.5, 10.0, 10.0, 2.0, 50.0, 50.0);
         d2d_transform(ds, 1.0, 0.0, 0.0, 0.5, 0.0, 0.0);
         d2d_fillrect(ds, 0.0, 0.0, 50.0, 50.0);
         test_img_hash(ds, "Transform", 0x17EA9B8F);
      }
      break;

      case ResetTransform:
      {
         d2d_translate(ds, 100.0, 100.0);
         d2d_resettransform(ds);
         d2d_fillrect(ds, 0.0, 0.0, 50.0, 50.0);
         test_img_hash(ds, "ResetTransform", 0x4A5BCCEB);
      }
      break;

      case CreateLinearGradient:
      {
         d2d_createlineargradient(ds, 1, 100.0, 100.0, 450.0, 450.0);
         d2d_addcolorstop(ds, 1, 0, "green");
         d2d_addcolorstop(ds, 1, 1, "cyan");
         d2d_setfillstylegradient(ds, 1);
         d2d_fillrect(ds, 50.0, 50.0, 500.0, 500.0);
         d2d_releaseid(ds, 1);

         test_img_hash(ds, "CreateLinearGradient", 0xF786D7FA);
      }
      break;

      case CreateRadialGradient:
      {
         d2d_createradialgradient(ds, 1, 100.0, 100.0, 50.0, 450.0, 450.0, 100.0);
         d2d_addcolorstop(ds, 1, 0, "green");
         d2d_addcolorstop(ds, 1, 1, "cyan");
         d2d_setfillstylegradient(ds, 1);
         d2d_fillrect(ds, 50.0, 50.0, 500.0, 500.0);
         d2d_releaseid(ds, 1);

         test_img_hash(ds, "CreateRadialGradient", 0x44CD7BC4);
      }
      break;

      case ImageDataAndPutImageData:
      {
         #define base_width 3
         #define base_height 3
         #define base_area base_width*base_height
         #define base_scale 64
         #define img_width base_width*base_scale
         #define img_height base_height*base_scale
         #define img_area img_width*img_height

         const unsigned long base_img[base_area] = {
            0xFFFF0000, 0xFF00FF00, 0xFF0000FF,
            0xFF00FF00, 0xFF0000FF, 0xFFFF0000,
            0xFF0000FF, 0xFFFF0000, 0xFF00FF00,
         };
         unsigned long* img_data = malloc(img_area * 4);
         for (int b_y = 0; b_y < base_height; b_y++) {
            for (int s_y = 0; s_y < base_scale; s_y++) {
               int img_y = (b_y*base_scale + s_y) * img_width;
               for (int b_x = 0; b_x < base_width; b_x++) {
                  for (int s_x = 0; s_x < base_scale; s_x++) {
                     int img_x = b_x*base_scale + s_x;
                     img_data[img_y + img_x] = base_img[b_y*base_width + b_x];
                  }
               }
            }
         }

         d2d_imagedata(ds, 1, (void*)img_data, img_area*4, img_width, img_height);
         d2d_putimagedata(ds, 1, 100, 100);
         d2d_releaseid(ds, 1);

         test_img_hash(ds, "ImageDataAndPutImageData", 0xD755D8A9);
         free(img_data);
      }
      break;

      case LoadAndDrawImage:
      {
         #ifdef ASYNC
         d2d_load_image("./test-img.jpg", 1);
         d2d_drawimage(ds, 1, 0.0, 0.0);
         d2d_releaseid(ds, 1);
         test_img_hash(ds, "LoadAndDrawImage", 0xF35DC5F0);
         #else
         printf("LoadAndDrawImage test can only be tested with async\n");
         #endif
      }
      break;
   }

   d2d_end_draw_sequence(ds);
}
void test_all() {
   for (int test_id = EmptyCanvas; test_id <= LoadAndDrawImage; test_id++) {
      test_case(test_id);
   }
}

void test_specific(int id) {
   test_case(id);
}