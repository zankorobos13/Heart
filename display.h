#include <vector>

class Display{
    public:
        void PrintString(String str){
            Serial.println("=================");
            Serial.println(str);
            Serial.println("=================");
        }

        void PrintImage(std::vector<bool> image){
            Serial.println("=================");
            for (int i = 0; i < image.size(); i++){
                if (image[i])
                    Serial.print(1);
                else
                    Serial.print(0);
            }
            Serial.println();
            Serial.println("=================");
        }
};