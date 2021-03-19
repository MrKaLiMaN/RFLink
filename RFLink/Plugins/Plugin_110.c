
#define PLUGIN_DESC_110 "TicPulses Mrk"

#define TIC_LENGTH_MIN 260
#define TIC_LENGTH_MAX 300

#define TIC_PULSE_MIN 300   // 300
#define TIC_PULSE_MID 512   // 512
#define TIC_PULSE_MAX 700   // 700

#ifdef PLUGIN_110

#include "../4_Display.h"
#include "../7_Utils.h"



boolean Plugin_110(byte function, const char *string)
{

   //==================================================================================
   // Perform pre sanity check
   //==================================================================================
   if (RawSignal.Number < TIC_LENGTH_MIN)
      return false;
   if (RawSignal.Number > TIC_LENGTH_MAX)
      return false;

   for (int x = 1; x < RawSignal.Number - 1 ; x++)
   {
      if (RawSignal.Pulses[x] < 100)
      {
         // Serial.print("pulse too short : ");
         // Serial.println(RawSignal.Pulses[x], DEC);
         return false; // pulse too short
      }
      else if (RawSignal.Pulses[x] > TIC_PULSE_MAX)
      {
         // Serial.print("pulse too long : ");
         // Serial.println(RawSignal.Pulses[x], DEC);
         return false; // pulse too long
      }
   }

   //==================================================================================
   //  Get all rf bits 
   //  24 bits for preamble x2 + sync 3 bytes--> 0xAA 0xAA 0x9C
   //  336 rf bits for manchester encoded payload (21 bytes x 8 bits x 2 (manchester) )
   //==================================================================================
   byte tic_message[45];
   byte bit_value;
   uint16_t position_in_byte = 0;
   uint16_t position_in_buffer = 0;
   int nb_rf_bits = 0;


   for (int x = 1; x <= RawSignal.Number; x++)
   {
      if((x % 2) == 1)
         bit_value = 0x01;    // HIGH STATE
      else
         bit_value = 0x00;    // LOW STATE


      // 1 rf bit
      if (RawSignal.Pulses[x] < TIC_PULSE_MIN)
         nb_rf_bits = 1;

      // 2 rf bits
      else if (RawSignal.Pulses[x] < TIC_PULSE_MID)
         nb_rf_bits = 2;

      // 3 rf bits
      else if (RawSignal.Pulses[x] < TIC_PULSE_MAX)
         nb_rf_bits = 3;
      else if ( (RawSignal.Pulses[x] >= TIC_PULSE_MAX) && (x == RawSignal.Number) )
         nb_rf_bits = 1;
      else
      {
         Serial.print("pulse too long #2 : ");
         Serial.println(RawSignal.Pulses[x], DEC);
         Serial.println(RawSignal.Number, DEC);
         return false;
      }

      // store rf bits
      for (int y = 0; y <  nb_rf_bits ; y++)
      {
         tic_message[position_in_buffer] <<= 1;
         tic_message[position_in_buffer] |= bit_value;
         position_in_byte ++;

         if ((position_in_byte % 8) == 0){
            position_in_buffer++;
         }
      }
   }


   //==================================================================================
   // Decode Manchester
   //==================================================================================
   byte payload[21];
   position_in_byte = 0;
   position_in_buffer = 0;

   for (int i = 3; i < 45 ; i++)
   {
      byte transition;

      for (int j = 6; j >= 0 ; j-=2)
      {
         transition = (tic_message[i] >> j )   & 0x03;

         if (transition == 0x01)          // 01 _-
            bit_value = 0x00;
         else if (transition == 0x02)     // 10 -_
            bit_value = 0x01;
         else
         {
            Serial.print("Not Manchester : ");
            Serial.print("tic_message ");
            Serial.print( i , DEC);
            Serial.print("/44 : ");
            Serial.print( tic_message[i] , HEX);
            Serial.println();
            
            return false;
         }
         // Serial.println(position_in_buffer, DEC);

         payload[position_in_buffer] <<= 1;
         payload[position_in_buffer] |= bit_value;
         position_in_byte ++;

         if ((position_in_byte % 8) == 0){
            position_in_buffer++;
         }
      }
   }

   // //==================================================================================
   // // Print preamble + sync
   // //==================================================================================
   // Serial.println("---------------------------------------------");
   // Serial.print("Preamble x2 + Sync : ");
   // for (int i = 0; i < 3 ; i++)
   // {
   //    Serial.print("0x");
   //    Serial.print(tic_message[i], HEX);
   //    Serial.print(" ");
   // }
   // Serial.println();

   // Serial.print("Manchester data : ");
   // for (int i = 3; i < 45 ; i++)
   // {
   //    Serial.print("0x");
   //    Serial.print(tic_message[i], HEX);
   //    Serial.print(" ");
   // }
   // Serial.println();
   // Serial.println("---------------------------------------------");
   // // return false;

   // //==================================================================================
   // // Print decoded Manchester
   // //==================================================================================
   // Serial.print("Manchester decoded : ");
   // for (int i = 0; i < 21 ; i++)
   // {
   //    Serial.print("0x");
   //    Serial.print(payload[i], HEX);
   //    Serial.print(" ");
   // }
   // Serial.println();
   // Serial.println("---------------------------------------------");


   //==================================================================================
   // Checksum
   //==================================================================================
   uint16_t crc_calc = payload[19];
   crc_calc <<= 8;
   crc_calc |= payload[20];

   if (crc_calc != (crc16(payload, 19, 0x1021, 0x1D0F) ^ 0xFFFF) )
   {
      Serial.println("Erreur CRC !!");
      return false;
   }

   
   //==================================================================================
   // Protocol Revision 1
   //==================================================================================
   // byte protocol = payload[1];

   //==================================================================================
   // Protocol Revision 2
   //==================================================================================
   // TODO 

   //==================================================================================
   // Protocol Revision 5 : Linky ?
   //==================================================================================
   // TODO 

   // TODO switch Ouput with Protocol Revision ? 1 2 5...


   //==================================================================================
   // Output
   //==================================================================================
/*
Coté owl 180 (sonde de mesure de courant Oregon) ça remonte deja correctement dans home assistant.
Les messages sont du type :

20;0A;OWL_CM180;ID=003D;WATT=0fd8;KWATT=0078afdf;
20;0B;OWL_CM180;ID=003D;WATT=0ff8;KWATT=0078afe4;
20;0C;OWL_CM180;ID=003D;WATT=0fd8;KWATT=0078aff5;
20;0D;OWL_CM180;ID=003D;WATT=0fe8;KWATT=0078b007;
20;0E;OWL_CM180;ID=003D;WATT=1008;KWATT=0078b014;

Tu devrais ressortir sous ce format ce serait nativement géré
*/
   uint8_t current_price = payload[7] & 0x0F;

   uint16_t current_power = payload[17];
   current_power <<= 8;
   current_power |= payload[16];

   uint8_t battery = (payload[18] & 0x01) ^ 0x01;

   
   // max 4 bytes with display_IDn...
   uint32_t counter_id;
   for (int i = 2; i <=5 ; i++)
   {
      counter_id <<= 8;
      counter_id |= payload[i];
   }

   // Index Heures Creuses
   uint32_t index1 = 0;
   for (int i = 11; i >=8 ; i--)
   {
      index1 <<= 8;
      index1 |= payload[i];
   }

   // Index Heures Pleines
   uint32_t index2 = 0;
   for (int i = 15; i >=12 ; i--)
   {
      index2 <<= 8;
      index2 |= payload[i];
   }

   display_Header();
   display_Name(PSTR("TIC_PULSES_V2"));

   // Counter Id (5 bytes in payload, 4 bytes with RFLink)
   display_IDn(counter_id , 4);

   // Current Price Time
   // display_SWITCH(current_price);

   // switch Current Price Time
   // if(current_price == 3)
   //    display_KWATT(index1);
   // else
   //    display_KWATT(index2);

   display_KWATT(index2);

   // display_SWITCH(0xA4);
   // display_WATT(current_power);
   display_BAT(battery);

   display_Footer();
   // ----------------------------------
   RawSignal.Repeats = true;
   RawSignal.Number = 0;
   return true;
 

}


#endif //PLUGIN_000