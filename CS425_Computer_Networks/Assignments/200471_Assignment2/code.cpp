#include  <iostream>
#include <string>
#include <bits/stdc++.h>
using namespace std;

int transmitLen, messageLen, frame_size;

int findRandom(int n)
{
   int num = ((int)rand() % n);
   return num;
}
 
string generate_Random_Binary_String(int N) {
   string s = "";
   for(int i=0; i<N; i++){
      s += to_string(findRandom(2));
   }

   return s;
}

string generateFrame(string data_block, string pattern){

   string frame = "";
   int p = pattern.length();

   for(int j = 0; j < messageLen; j++){
      if(data_block[j] == '1'){
         for(int i = j; i < (j+p); i++){
            if((data_block[i] == '1' && pattern[i-j] == '1') || (data_block[i] == '0' && pattern[i-j] == '0')){
               data_block[i] = '0';
            } else {
               data_block[i] = '1';
            }
         }
      }
   }

   for(int j = messageLen; j<data_block.length(); j++){
      frame += data_block[j];
   }

   return frame;
}

string generateCRC(string data_block, string frame){

   return data_block + frame;
}

string generateErrorData(string CRC) {
   string error_pattern = generate_Random_Binary_String(transmitLen);
   cout << "Error Pattern(Randomly Generated): " << error_pattern << "\n";
   string received_data = CRC;

   for(int i=0; i<error_pattern.length(); i++){
      if(error_pattern[i] == '1'){
         if(received_data[i] == '1')
            received_data[i] = '0';
         else
            received_data[i] = '1';
      } 
   }

   return received_data;
}

void checkReceivedData(string rdata, string pattern){
  

   int p = pattern.length();

   for(int j = 0; j < messageLen; j++){
      if(rdata[j] == '1'){
         for(int i = j; i < (j+p); i++){
            if((rdata[i] == '1' && pattern[i-j] == '1') || (rdata[i] == '0' && pattern[i-j] == '0')){
               rdata[i] = '0';
            } else {
               rdata[i] = '1';
            }
         }
      }
   }

   for(int j = messageLen; j<rdata.length(); j++){
      if(rdata[j] == '1')
         cout << "DISCARDED\n";
         return;
   }

   cout << "ACCEPTED\n";
   return;
}

int main(){

   srand(time(0));
   string data_block, dup_data_block, pattern;

   cout << "Enter Data Size(k): ";
   cin >> messageLen;

   cout << "Enter Pattern (P): ";
   cin >> pattern;

   // Generate message
   data_block = generate_Random_Binary_String(messageLen);
   cout << "\n";
   cout << "----------RESULTS------------\n";
   cout << "\n";
   cout << "Data Block(Randomly Generated): " << data_block << "\n";
   dup_data_block = data_block;

   // Find sizes
   frame_size = pattern.length() - 1;
   transmitLen = messageLen + frame_size;

   // Append data_block with 0
   for (int j = data_block.length(); j < transmitLen; j++){
         data_block += '0';
   }

   string frame = generateFrame(data_block, pattern);
   string CRC = generateCRC(dup_data_block, frame);
   cout << "CRC(Data Sent): " << CRC << "\n";
   checkReceivedData(CRC, pattern);

   string received_data = generateErrorData(CRC);
   cout << "Corrupted Received Data: " <<received_data << "\n";
   checkReceivedData(received_data, pattern);   
   
   return 0;
}