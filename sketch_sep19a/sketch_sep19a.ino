
#define MEGA 1000000

float calculateSpeed() 
{
  unsigned long int t1, t2;
  float speed_mps = 0;
  short n = 5
  int distance;

  for (int i = 0; i < n; i++) 
  {
    t1 = micros();
    distance = caluclateDistance();
    t2 = micros();
    speed_mps += ((float) distance / (float) (t2 - t1)) * ((float) MEGA / (float) n);
  }
  
  
}
