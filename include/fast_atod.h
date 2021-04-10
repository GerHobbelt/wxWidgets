#pragma once

// this is the same signature as from_chars (which doesn't work for float on gcc/clang)
// ie it is a [start, end)  (not including *end). Well suited to parsing read only memorymappedfile
inline double fast_atod(const char *num, const char *end = nullptr)
{
   if (!end) {
      end = num + strlen(num);
   }
   if (!num || end <= num) {
      return 0;
   }

   int sign = 1;
   double int_part = 0.0;
   double frac_part = 0.0;
   bool has_frac = false;
   bool has_exp = false;

   // +/- sign
   if (*num == '-') {
      ++num;
      sign = -1;
   }
   else if (*num == '+') {
      ++num;
   }

   while (num != end) {
      if (*num >= '0' && *num <= '9') {
         int_part = int_part * 10 + (*num - '0');
      }
      else if (*num == '.') {
         has_frac = true;
         ++num;
         break;
      }
      else if (*num == 'e') {
         has_exp = true;
         ++num;
         break;
      }
      else {
         return sign * int_part;
      }
      ++num;
   }

   if (has_frac) {
      double frac_exp = 0.1;

      while (num != end) {
         if (*num >= '0' && *num <= '9') {
            frac_part += frac_exp * (*num - '0');
            frac_exp *= 0.1;
         }
         else if (*num == 'e') {
            has_exp = true;
            ++num;
            break;
         }
         else {
            return sign * (int_part + frac_part);
         }
         ++num;
      }
   }

   // parsing exponent part
   double exp_part = 1.0;
   if (num != end && has_exp) {
      int exp_sign = 1;
      if (*num == '-') {
         exp_sign = -1;
         ++num;
      }
      else if (*num == '+') {
         ++num;
      }

      int e = 0;
      while (num != end && *num >= '0' && *num <= '9') {
         e = e * 10 + *num - '0';
         ++num;
      }

      auto pow10 = [](int n) {
         double ret = 1.0;
         double r = 10.0;
         if (n < 0) {
            n = -n;
            r = 0.1;
         }

         while (n) {
            if (n & 1) {
               ret *= r;
            }
            r *= r;
            n >>= 1;
         }
         return ret;
      };

      exp_part = pow10(exp_sign * e);
   }

   return sign * (int_part + frac_part) * exp_part;
}
