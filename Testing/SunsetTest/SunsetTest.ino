/*Test using this library
 * https://github.com/dmkishi/Dusk2Dawn
*/

#include <Dusk2Dawn.h>
#include <Time.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();
  Serial.println("Serial started");
  
  Dusk2Dawn toronto(43.666468, -79.477260, -5);

  int sunset = toronto.sunset(2018,01,13,false);
  Serial.println(sunset);

  char time[6];
  Dusk2Dawn::min2str(time, sunset);
  Serial.println(time); // 06:58
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
