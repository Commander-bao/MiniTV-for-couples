#include <ArduinoJson.h> //请使用ArduinoJson V5版本，最新版V6会导致编译失败
#include <TimeLib.h>
#include <Preferences.h> //用于保存数据
Preferences preferences;
String PrefSSID, PrefPassword, City;

#include <TFT_eSPI.h> //tft显示屏功能
#include <SPI.h>
#include <EEPROM.h> //带电可擦除可编程只读存储器
#include <TJpg_Decoder.h> //JPG图像显示库
/*使用Tjpg_Decoder库创建C数组图片的网址: http://tom为eko.net/online_tools/file_to_hex.php?lang=en*/

#include <WiFi.h> //使用WiFi功能
#include <WiFiUdp.h>
#include <HTTPClient.h>

/*配网文件*/
#include "SetWiFi.h"

/*引用字体*/
#include "../font/ZdyLwFont_20.h"
#include "../font/FxLED_32.h"

/*引用图片*/
#include "../img/setWiFi_img.h"
#include "../img/humidity.h"
#include "../img/temperature.h"
#include "../img/wififalse.h"
#include "../img/weather_code.h"
#include "../img/astronaut.h"
#include "../img/singing.h"
#include "../img/love.h"
#include "../img/dancing.h"
#include "../img/battery.h"

/*基础配置*/
#define Baudrate 115200 //定义串口波特率
#define ScreenRotation 0 //设置屏幕显示方向

/*硬件引脚配置*/
#define leftButton 13   //左按钮
#define rightButton 15  //右按钮
#define battery 34       //电压检测

TFT_eSPI tft = TFT_eSPI(); //屏幕对象实例化
TFT_eSprite clk = TFT_eSprite(&tft);

uint32_t targetTime = 0;   
uint16_t bgColor = 0xFFFF;      //屏幕背景色
String cityCode = "101230201";  //天气城市代码

//NTP服务器
static const char ntpServerName[] = "ntp6.aliyun.com";
const int timeZone = 8;     //东八区

WiFiUDP Udp;
unsigned int localPort = 8000;

time_t getNtpTime();
String num2str(int digits);
void sendNTPpacket(IPAddress &address);

uint16_t weatherCode = 99;  //根据数值决定显示那种天气图标，在weatherData()函数中被使用
bool getCityWeaterFlag = false;

byte Mode = 1;
byte flag = 0;

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  if ( y >= tft.height() ) return 0;
  tft.pushImage(x, y, w, h, bitmap); //显示图片
  return 1;  // Return 1 to decode next block
}

byte loadNum = 6;
void loading(byte delayTime,byte NUM){
  clk.setColorDepth(8);
  clk.createSprite(200, 50);
  clk.fillSprite(0x0000);
  clk.loadFont(ZdyLwFont_20); //加载font/ZdyLwFont_20字体
  clk.drawRoundRect(0,0,200,16,8,0xFFFF);
  clk.fillRoundRect(3,3,loadNum,10,5,0xFFFF);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_WHITE, 0x0000); 
  clk.drawString("正在连接 "+ PrefSSID + " ...",100,40,2);
  clk.pushSprite(20,110);
  clk.deleteSprite();
  loadNum += NUM;
  if(loadNum>=194){
    loadNum = 194;
  }
  delay(delayTime);
  clk.unloadFont(); //释放加载字体资源
}

//显示wifi连接失败，并重新进入配网模式
void displayConnectWifiFalse() {
  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);
  TJpgDec.drawJpg(0,0,wififalse, sizeof(wififalse)); 
  delay(5000);
}

unsigned long oldTime_1 = 0;
int imgNum_1 = 0;
int connectTimes = 0;
int lightValue=0,backLight_hour=0;

int Filter_Value;

long __tstamp;
char m[2] = {'0', '\0'};
boolean checkMillis(int m) {
  if (millis() - __tstamp > m) {
    __tstamp = millis();
    return true;
  } else {
    return false;
  }
}

//强制门户Web配网
bool setWiFi_Flag = false;
void setWiFi() {  //设置WiFi
  TJpgDec.drawJpg(0,0,setWiFi_img, sizeof(setWiFi_img));

  initBasic();
  initSoftAP();
  initWebServer();
  initDNS();
  while(setWiFi_Flag == false) {
    server.handleClient();
    dnsServer.processNextRequest();
    if(WiFi.status() == WL_CONNECTED) {
      server.stop();
      setWiFi_Flag = true;
    }
  }
}

time_t prevDisplay = 0; // 显示时间
unsigned long weaterTime = 0;
String scrollText[6];
String ButtonScrollText[8];

//天气信息写到屏幕上
void weatherData(String *cityDZ,String *dataSK,String *dataFC,String *dataSuggest){
  
  DynamicJsonDocument doc(4096);
  deserializeJson(doc, *dataSK);
  JsonObject sk = doc.as<JsonObject>();

  //TFT_eSprite clkb = TFT_eSprite(&tft);
  
  /***绘制相关文字***/
  clk.setColorDepth(8);
  clk.loadFont(ZdyLwFont_20); //加载font/ZdyLwFont_20字体
  
  //温度
  clk.createSprite(54, 32); 
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor); 
  clk.drawString(sk["temp"].as<String>()+"℃",27,16);
  clk.pushSprite(185,168);
  clk.deleteSprite();
 
  //城市名称
  clk.createSprite(88, 32);  //88,32
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor); 
  //clk.drawString(sk["cityname"].as<String>()+"区",44,18);
  clk.drawString(sk["cityname"].as<String>(),44,18);
  clk.pushSprite(151,1);
  clk.deleteSprite();
  
  //PM2.5空气指数
  uint16_t pm25BgColor;//优
  String aqiTxt;
  int pm25V = sk["aqi"];
  //Serial.println("pm25V:" + String(pm25V));
  if(pm25V >= 301) {
    pm25BgColor = tft.color565(255,36,0);//重度
    aqiTxt = "严重";
  }else if(pm25V >= 201 & pm25V <= 300){
    pm25BgColor = tft.color565(136,11,32);//重度
    aqiTxt = "重度";
  }else if(pm25V >= 151 & pm25V <= 200){
    pm25BgColor = tft.color565(186,55,121);//中度
    aqiTxt = "中度";
  }else if(pm25V >= 101 & pm25V <= 160){
    pm25BgColor = tft.color565(242,159,57);//轻
    aqiTxt = "轻度";
  }else if(pm25V >= 51 & pm25V <= 100){
    pm25BgColor = tft.color565(247,219,100);//良
    aqiTxt = "良";
  }else if(pm25V >= 0 & pm25V <= 50) {
    pm25BgColor = tft.color565(156,202,127);//优
    aqiTxt = "优";
  }
  clk.createSprite(50, 24); 
  clk.fillSprite(bgColor);
  clk.fillRoundRect(0,0,50,24,4,pm25BgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(0xFFFF); 
  clk.drawString(aqiTxt,25,13);
  clk.pushSprite(5,140);
  clk.deleteSprite();

  //湿度
  clk.createSprite(56, 24); 
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor); 
  clk.drawString(sk["SD"].as<String>(),28,13);
  clk.pushSprite(185,140);
  clk.deleteSprite();

  //左上角滚动字幕
  //解析第二段JSON
  scrollText[0] = "实时天气 "+sk["weather"].as<String>();
  scrollText[1] = "空气质量 "+aqiTxt;
  
  scrollText[2] = "风向 "+sk["WD"].as<String>()+sk["WS"].as<String>();
  
  deserializeJson(doc, *cityDZ);
  JsonObject dz = doc.as<JsonObject>();
  scrollText[3] = "今日 "+dz["weather"].as<String>();

  //显示天气图标
  String weatherCodeText = dz["weathercode"].as<String>();
  weatherCode = weatherCodeText.substring(1,weatherCodeText.length()+1).toInt();

  switch(weatherCode) {
    case 0:TJpgDec.drawJpg(10,105,d00_40X30, sizeof(d00_40X30));break;
    case 1:TJpgDec.drawJpg(10,105,d01_40X30, sizeof(d01_40X30));break;
    case 2:TJpgDec.drawJpg(10,105,d02_40X30, sizeof(d02_40X30));break;
    case 3:TJpgDec.drawJpg(10,105,d03_40X30, sizeof(d03_40X30));break;
    case 4:TJpgDec.drawJpg(10,105,d04_40X30, sizeof(d04_40X30));break;
    case 5:TJpgDec.drawJpg(10,105,d05_40X30, sizeof(d05_40X30));break;
    case 6:TJpgDec.drawJpg(10,105,d06_40X30, sizeof(d06_40X30));break;
    case 7:TJpgDec.drawJpg(10,105,d07_40X30, sizeof(d07_40X30));break;
    case 8:TJpgDec.drawJpg(10,105,d08_40X30, sizeof(d08_40X30));break;
    case 9:TJpgDec.drawJpg(10,105,d09_40X30, sizeof(d09_40X30));break;
    case 10:TJpgDec.drawJpg(10,105,d10_40X30, sizeof(d10_40X30));break;
    case 11:TJpgDec.drawJpg(10,105,d11_40X30, sizeof(d11_40X30));break;
    case 12:TJpgDec.drawJpg(10,105,d12_40X30, sizeof(d12_40X30));break;
    case 13:TJpgDec.drawJpg(10,105,d13_40X30, sizeof(d13_40X30));break;
    case 14:TJpgDec.drawJpg(10,105,d14_40X30, sizeof(d14_40X30));break;
    case 15:TJpgDec.drawJpg(10,105,d15_40X30, sizeof(d15_40X30));break;
    case 16:TJpgDec.drawJpg(10,105,d16_40X30, sizeof(d16_40X30));break;
    case 17:TJpgDec.drawJpg(10,105,d17_40X30, sizeof(d17_40X30));break;
    case 18:TJpgDec.drawJpg(10,105,d18_40X30, sizeof(d18_40X30));break;
    case 19:TJpgDec.drawJpg(10,105,d19_40X30, sizeof(d19_40X30));break;
    case 20:TJpgDec.drawJpg(10,105,d20_40X30, sizeof(d20_40X30));break;
    case 21:TJpgDec.drawJpg(10,105,d21_40X30, sizeof(d21_40X30));break;
    case 22:TJpgDec.drawJpg(10,105,d22_40X30, sizeof(d22_40X30));break;
    case 23:TJpgDec.drawJpg(10,105,d23_40X30, sizeof(d23_40X30));break;
    case 24:TJpgDec.drawJpg(10,105,d24_40X30, sizeof(d24_40X30));break;
    case 25:TJpgDec.drawJpg(10,105,d25_40X30, sizeof(d25_40X30));break;
    case 26:TJpgDec.drawJpg(10,105,d26_40X30, sizeof(d26_40X30));break;
    case 27:TJpgDec.drawJpg(10,105,d27_40X30, sizeof(d27_40X30));break;
    case 28:TJpgDec.drawJpg(10,105,d28_40X30, sizeof(d28_40X30));break;
    case 29:TJpgDec.drawJpg(10,105,d29_40X30, sizeof(d29_40X30));break;
    case 30:TJpgDec.drawJpg(10,105,d30_40X30, sizeof(d30_40X30));break;
    case 31:TJpgDec.drawJpg(10,105,d31_40X30, sizeof(d31_40X30));break;
    case 32:TJpgDec.drawJpg(10,105,d32_40X30, sizeof(d32_40X30));break;
    case 33:TJpgDec.drawJpg(10,105,d33_40X30, sizeof(d33_40X30));break;
    case 49:TJpgDec.drawJpg(10,105,d49_40X30, sizeof(d49_40X30));break;
    case 53:TJpgDec.drawJpg(10,105,d53_40X30, sizeof(d53_40X30));break;
    case 54:TJpgDec.drawJpg(10,105,d54_40X30, sizeof(d54_40X30));break;
    case 55:TJpgDec.drawJpg(10,105,d55_40X30, sizeof(d55_40X30));break;
    case 56:TJpgDec.drawJpg(10,105,d56_40X30, sizeof(d56_40X30));break;
    case 57:TJpgDec.drawJpg(10,105,d57_40X30, sizeof(d57_40X30));break;
    case 58:TJpgDec.drawJpg(10,105,d58_40X30, sizeof(d58_40X30));break;
    case 301:TJpgDec.drawJpg(10,105,d301_40X30, sizeof(d301_40X30));break;
    case 302:TJpgDec.drawJpg(10,105,d302_40X30, sizeof(d302_40X30));break;
    default:break;
  }
  
  deserializeJson(doc, *dataFC);
  JsonObject fc = doc.as<JsonObject>();
  
  scrollText[4] = "最低温度 "+fc["fd"].as<String>()+"℃";
  scrollText[5] = "最高温度 "+fc["fc"].as<String>()+"℃";

  clk.unloadFont(); //释放加载字体资源

  deserializeJson(doc, *dataSuggest);
  JsonObject dataSuggestJson = doc.as<JsonObject>();
  ButtonScrollText[0] = dataSuggestJson["lk_name"].as<String>() + " " + dataSuggestJson["lk_hint"].as<String>();
  ButtonScrollText[1] = dataSuggestJson["cl_name"].as<String>() + " " + dataSuggestJson["cl_hint"].as<String>();
  ButtonScrollText[2] = dataSuggestJson["uv_name"].as<String>() + " " + dataSuggestJson["uv_hint"].as<String>();
  ButtonScrollText[3] = dataSuggestJson["ct_name"].as<String>() + " " + dataSuggestJson["ct_hint"].as<String>();
  ButtonScrollText[4] = dataSuggestJson["gm_name"].as<String>() + " " + dataSuggestJson["gm_hint"].as<String>();
  ButtonScrollText[5] = dataSuggestJson["ys_name"].as<String>() + " " + dataSuggestJson["ys_hint"].as<String>();
  ButtonScrollText[6] = dataSuggestJson["co_name"].as<String>() + " " + dataSuggestJson["co_hint"].as<String>();
}

// 获取城市天气

void getCityWeater(){
  HTTPClient httpClient;
  while(getCityWeaterFlag == false) {
    String URL = "http://d1.weather.com.cn/weather_index/" + cityCode + ".html?_="+String(now());
    //创建 HTTPClient 对象
    
    httpClient.begin(URL); 
    
    //设置请求头中的User-Agent
    httpClient.setUserAgent("Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Mobile/15A372 Safari/604.1");
    httpClient.addHeader("Referer", "http://www.weather.com.cn/");
  
    //启动连接并发送HTTP请求
    int httpCode = httpClient.GET();
    Serial.println("正在获取天气数据");
    //Serial.println(URL);

    //如果服务器响应OK则从服务器获取响应体信息并通过串口输出
    if (httpCode == HTTP_CODE_OK) {

      String str = httpClient.getString();
      Serial.println(str);

      int indexStart = str.indexOf("weatherinfo\":");
      int indexEnd = str.indexOf("};var alarmDZ");

      String jsonCityDZ = str.substring(indexStart+13,indexEnd);
      //Serial.println(jsonCityDZ);

      indexStart = str.indexOf("dataSK =");
      indexEnd = str.indexOf(";var dataZS");
      String jsonDataSK = str.substring(indexStart+8,indexEnd);
      //Serial.println(jsonDataSK);

      indexStart = str.indexOf("\"f\":[");
      indexEnd = str.indexOf(",{\"fa");
      String jsonFC = str.substring(indexStart+5,indexEnd);
      //Serial.println(jsonFC);

      indexStart = str.indexOf(";var dataZS ={\"zs\":");
      indexEnd = str.indexOf(",\"cn\":\"厦门\"};var fc =");
      String jsonSuggest = str.substring(indexStart+19,indexEnd);
      //Serial.println(jsonSuggest);
      
      weatherData(&jsonCityDZ,&jsonDataSK,&jsonFC,&jsonSuggest);
      Serial.println("天气数据获取成功");
      getCityWeaterFlag = true;
    } else {
      Serial.print("请求城市天气错误：");
      Serial.println(String(httpCode) + "正在重新获取...");
    }
  }
  //关闭ESP32与服务器连接
  httpClient.end();
}

int currentIndex = 0;
int prevTime = 0;
TFT_eSprite clkb = TFT_eSprite(&tft);

void scrollTxt(int pos){
    clkb.createSprite(148, 24); 
    clkb.fillSprite(bgColor);
    clkb.setTextWrap(false);
    clkb.setTextDatum(CC_DATUM);
    clkb.setTextColor(TFT_BLACK, bgColor); 
    clkb.drawString(scrollText[currentIndex],74,pos+14);
    clkb.pushSprite(2,4);
}

void scrollBanner(){
  if(millis() - prevTime > 3500){ //3.5秒切换一次

    if(scrollText[currentIndex]){
  
      clkb.setColorDepth(8);
      clkb.loadFont(ZdyLwFont_20);
      
      for(int pos = 20; pos>0 ; pos--){
        scrollTxt(pos);
      }
      
      clkb.deleteSprite();
      clkb.unloadFont();
  
      if(currentIndex>=5){
        currentIndex = 0;  //回第一个
      }else{
        currentIndex += 1;  //准备切换到下一个  
      }
      
      //Serial.println(currentIndex);
      
    }
    prevTime = millis();
    
  }
}

/*底部生活信息滚动显示 */

byte ButtoncurrentIndex = 0;
unsigned long ButtonprevTime = 0;
TFT_eSprite clkbb = TFT_eSprite(&tft);

void ButtonScrollTxt(int pos){
  clkbb.createSprite(240, 40); 
  clkbb.fillSprite(bgColor);
  clkbb.setTextDatum(CC_DATUM);
  clkbb.setTextColor(TFT_BLACK, bgColor); 
  clkbb.drawString(ButtonScrollText[ButtoncurrentIndex],120,pos+20);
  clkbb.pushSprite(0,201);
}

void ButtonscrollBanner(){
  if(millis() - ButtonprevTime > 5000){ //5秒切换一次

    if(ButtonScrollText[ButtoncurrentIndex]){
      clkbb.loadFont(ZdyLwFont_20);
      
      for(int pos = 20; pos>0 ; pos--){
        ButtonScrollTxt(pos);
      }
      
      clkbb.deleteSprite();
      clkbb.unloadFont();
  
      if(ButtoncurrentIndex >= 6){
        ButtoncurrentIndex = 0;  //回第一个
      }else{
        ButtoncurrentIndex += 1;  //准备切换到下一个  
      } 
    }
    ButtonprevTime = millis();
  }
}

unsigned long oldTime = 0,imgNum = 1;
/*动画播放*/
void imgDisplay(){
  int x=75,y=94,dt;
  switch(Mode){
    case 1: dt = 100;break;
    case 2: dt = 50;break;
    case 3: dt = 50;break;
    case 4: dt = 50;break;
  }
  dt = 100;
  if(millis() - oldTime >= dt) {
    imgNum = imgNum + 1;
    oldTime = millis();
  }
  if(Mode == 1) { //动画-太空人
    switch(imgNum) {
      case 1: TJpgDec.drawJpg(x,y,i0, sizeof(i0));break;
      case 2: TJpgDec.drawJpg(x,y,i1, sizeof(i1));break;
      case 3: TJpgDec.drawJpg(x,y,i2, sizeof(i2));break;
      case 4: TJpgDec.drawJpg(x,y,i3, sizeof(i3));break;
      case 5: TJpgDec.drawJpg(x,y,i4, sizeof(i4));break;
      case 6: TJpgDec.drawJpg(x,y,i5, sizeof(i5));break;
      case 7: TJpgDec.drawJpg(x,y,i6, sizeof(i6));break;
      case 8: TJpgDec.drawJpg(x,y,i7, sizeof(i7));break;
      case 9: TJpgDec.drawJpg(x,y,i8, sizeof(i8));break;
      case 10: TJpgDec.drawJpg(x,y,i9, sizeof(i9));imgNum=1;break;
      default: imgNum = 1;break;
    }
  }else if(Mode == 2) {
    switch(imgNum) {
      case 1: TJpgDec.drawJpg(x,y,singing0, sizeof(singing0));break;
      case 2: TJpgDec.drawJpg(x,y,singing1, sizeof(singing1));break;
      case 3: TJpgDec.drawJpg(x,y,singing2, sizeof(singing2));break;
      case 4: TJpgDec.drawJpg(x,y,singing3, sizeof(singing3));imgNum=1;break;
      default: imgNum = 1;break;
    }
  }else if(Mode == 3) {
    switch(imgNum) {
      case 1: TJpgDec.drawJpg(x,y,love0, sizeof(love0));break;
      case 2: TJpgDec.drawJpg(x,y,love1, sizeof(love1));break;
      case 3: TJpgDec.drawJpg(x,y,love2, sizeof(love2));break;
      case 4: TJpgDec.drawJpg(x,y,love3, sizeof(love3));imgNum=1;break;
      default: imgNum = 1;break;
    }
  }else if(Mode == 4){
    switch(imgNum) {
      case 1: TJpgDec.drawJpg(x,y,dancing0, sizeof(dancing0));break;
      case 2: TJpgDec.drawJpg(x,y,dancing1, sizeof(dancing1));break;
      case 3: TJpgDec.drawJpg(x,y,dancing2, sizeof(dancing2));break;
      case 4: TJpgDec.drawJpg(x,y,dancing3, sizeof(dancing3));break;
      case 5: TJpgDec.drawJpg(x,y,dancing4, sizeof(dancing4));break;
      case 6: TJpgDec.drawJpg(x,y,dancing5, sizeof(dancing5));break;
      case 7: TJpgDec.drawJpg(x,y,dancing6, sizeof(dancing6));break;
      case 8: TJpgDec.drawJpg(x,y,dancing7, sizeof(dancing7));imgNum=1;break;
      default: imgNum = 1;break;
    }
  }
}

/*电量显示*/
void Battery(){
  uint16_t batteryLevel = analogRead(34);
  if(batteryLevel >= 1050){
    TJpgDec.drawJpg(190,113,battery0, sizeof(battery0));
  }else if(batteryLevel >= 970){
    TJpgDec.drawJpg(190,113,battery1, sizeof(battery1));
  }else if(batteryLevel >= 890){
    TJpgDec.drawJpg(190,113,battery2, sizeof(battery2));
  }else{
    TJpgDec.drawJpg(190,113,battery3, sizeof(battery3));
  }
}

//星期
String week(){
  String wk[7] = {"日","一","二","三","四","五","六"};
  String s = "周" + wk[weekday()-1];
  return s;
}

//月日
String monthDay(){
  String s = String(month());
  s = s + "月" + day() + "日";
  return s;
}

//时分
String hourMinute(){
  String s = num2str(hour());
  backLight_hour = s.toInt();
  s = s + ":" + num2str(minute());
  return s;
}

void digitalClockDisplay()
{
  
  clk.setColorDepth(8);

  /***中间时间区***/
  //时分
  clk.createSprite(140, 48);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor);
  clk.drawString(hourMinute(),70,24,7); //绘制时和分
  clk.pushSprite(28,40);
  clk.deleteSprite();
  
  //秒
  clk.createSprite(40, 32);
  clk.fillSprite(bgColor);
  
  clk.loadFont(FxLED_32);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor); 
  clk.drawString(num2str(second()),20,16);
  
  clk.unloadFont();
  clk.pushSprite(170,60);
  clk.deleteSprite();
  /***中间时间区***/

  /***底部***/
  clk.loadFont(ZdyLwFont_20);
  clk.createSprite(58, 32);
  clk.fillSprite(bgColor);

  //星期
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor);
  clk.drawString(week(),29,16);
  clk.pushSprite(1,168);
  clk.deleteSprite();
  
  //月日
  clk.createSprite(98, 32);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor);  
  clk.drawString(monthDay(),49,16);
  clk.pushSprite(61,168);
  clk.deleteSprite();
  
  clk.unloadFont();
  /***底部***/
}

String num2str(int digits)
{
  String s = "";
  if (digits < 10)
    s = s + "0";
  s = s + digits;
  return s;
}


/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP时间在消息的前48字节中
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  //Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  //Serial.print(ntpServerName);
  //Serial.print(": ");
  //Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      //Serial.println(secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR);
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // 无法获取时间时返回0
}

// 向NTP服务器发送请求
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

void leftButtonCallBack(void){  //左按钮中断回调函数
  flag ++;
  if(flag == 2){
    ESP.restart(); //按第二次时重启
  }
  Serial.println(flag);  
}

void rightButtonCallBack(void){ //右按钮中断回调函数
  if(Mode == 4){
    Mode = 1;
  }else{
    Mode ++;
  }
  Serial.println(Mode);
}

void setup(){
  Serial.begin(Baudrate); //设置串口波特率

  pinMode(leftButton, INPUT_PULLUP);   //将左按钮设置为内部上拉
  pinMode(rightButton, INPUT_PULLUP);  //将右按钮设置为内部上拉

  EEPROM.begin(8);

  // EEPROM.write(0,1);
  // EEPROM.commit();
  Mode = EEPROM.read(0);

  //tft.setSwapBytes(true); //使图片颜色由RGB->BGR
  tft.init(); //显示屏初始化
  tft.setRotation(ScreenRotation); //设置屏幕方向
  tft.fillScreen(TFT_WHITE);

  /*首次使用自动进入配网模式*/
  preferences.begin("wifi", false);
  PrefSSID =  preferences.getString("ssid", "none");
  PrefPassword =  preferences.getString("password", "none");
  preferences.end();
  if( PrefSSID == "none" ){
    setWiFi();
  }

  attachInterrupt(leftButton, leftButtonCallBack, RISING);    //将左按钮设置为上升沿触发
  attachInterrupt(rightButton, rightButtonCallBack, RISING);  //将右按钮设置为上升沿触发
  
  tft.fillScreen(0x0000);
  tft.setTextColor(TFT_BLACK, bgColor);

  targetTime = millis() + 1000; 

  Serial.println("正在连接"+ PrefSSID + " ...");
  WiFi.begin(PrefSSID.c_str(), PrefPassword.c_str());
  
  while (WiFi.status() != WL_CONNECTED) {
    for(byte n=0;n<10;n++){ 
      loading(100,1);
      connectTimes++;
      if(connectTimes >= 190) { //进度条即将结束时还未连接成功，则提示wifi连接失败，自动进入配网模式
        connectTimes = 0;
        displayConnectWifiFalse();
        setWiFi();
      }
    }
  }

  while(loadNum < 194 & connectTimes <= 189){ //让动画走完
    loading(0,5);
    connectTimes = 0;
  }

  Udp.begin(localPort);
  setSyncProvider(getNtpTime);
  setSyncInterval(300);

  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);

  delay(1000);

  tft.fillScreen(0x0000);
  tft.fillRoundRect(0,0,240,240,0,bgColor);//实心矩形
  /*绘制线框*/ 
  tft.drawFastHLine(0,0,240,TFT_BLACK);
  tft.drawFastHLine(0,34,240,TFT_BLACK);
  tft.drawFastHLine(0,200,240,TFT_BLACK);
  tft.drawFastVLine(150,0,34,TFT_BLACK);
  tft.drawFastHLine(0,166,240,TFT_BLACK);
  tft.drawFastVLine(60,166,34,TFT_BLACK);
  tft.drawFastVLine(160,166,34,TFT_BLACK);

  getCityWeater();

  Battery();
  TJpgDec.drawJpg(165,171,temperature, sizeof(temperature));  //温度图标
  TJpgDec.drawJpg(163,140,humidity, sizeof(humidity));  //湿度图标
}

void loop(){

  if (now() != prevDisplay) {
    prevDisplay = now();
    digitalClockDisplay();
  }

  //更新时，网络环境差的情况下，屏幕会有短暂停止刷新过程，网络环境好，该过程不明显，很难看出差别
  if(millis() - weaterTime > 60000){ //60000 1分钟更新一次天气
    getCityWeaterFlag = false;
    weaterTime = millis();
    getCityWeater();
    Battery();
    EEPROM.write(0,Mode);
    EEPROM.commit();
  }
  scrollBanner();
  ButtonscrollBanner();
  imgDisplay();
  //左按钮被按下时，进入配网模式
  if(flag == 1){
    setWiFi(); //按第一次时配网
  }
}
