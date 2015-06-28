//********************************************************************************
// Web Interface init
//********************************************************************************
void WebServerInit()
{
  // Prepare webserver pages
  WebServer.on("/", handle_root);
  WebServer.on("/config", handle_config);
  WebServer.on("/devices", handle_devices);
  WebServer.on("/hardware", handle_hardware);
  WebServer.on("/json.htm", handle_json);
  #ifdef ESP_CONNEXIO
    WebServer.on("/eventlist", handle_eventlist);
  #endif
  WebServer.on("/log", handle_log);
  WebServer.on("/debug", handle_debug);
  WebServer.begin();
}

//********************************************************************************
// Web Interface root page
//********************************************************************************
void handle_root() {
  Serial.print(F("HTTP : Webrequest : "));
  String webrequest = WebServer.arg("cmd");
  webrequest.replace("%20", " ");
  Serial.println(webrequest);
  char command[80];
  command[0] = 0;
  webrequest.toCharArray(command, 79);

  if (strcasecmp(command, "wifidisconnect") != 0)
  {
    String reply = "";
    printToWeb=true;
    printWebString = "";
    #ifdef ESP_CONNEXIO
      ExecuteLine(command,VALUE_SOURCE_SERIAL);
    #endif
    #ifdef ESP_EASY
      ExecuteCommand(command);
    #endif

    reply += printWebString;

    reply += F("<body><form>Welcome to ESP ");
    #ifdef ESP_CONNEXIO
      reply += F("Connexio : ");
    #endif
    #ifdef ESP_EASY
      reply += F("Easy : ");
    #endif
    reply += Settings.Name;
    
    reply += F("<BR><a href='/config'>Config</a>");
    reply += F("<BR><a href='/devices'>Devices</a>");
    reply += F("<BR><a href='/hardware'>Hardware</a>");
    reply += F("<BR><a href='/?cmd=reboot'>Reboot</a>");
    reply += F("<BR><a href='/?cmd=wifidisconnect'>Disconnect</a>");
    reply += F("<BR><a href='/?cmd=wificonnect'>Connect</a>");
    #ifdef ESP_CONNEXIO
      reply += F("<BR><a href='/eventlist'>Eventlist</a>");
    #endif
    reply += F("<BR><a href='/log'>Log</a>");
    reply += F("<BR><a href='/debug'>Debug</a>");
    reply += F("<BR><BR>System Info");

    IPAddress ip = WiFi.localIP();
    IPAddress gw = WiFi.gatewayIP();

    reply += F("<BR><BR>Uptime : ");
    reply += wdcounter/2;
    reply += F(" minutes");
    
    char str[20];
    sprintf(str, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
    reply += F("<BR>IP : ");
    reply += str;

    sprintf(str, "%u.%u.%u.%u", gw[0], gw[1], gw[2], gw[3]);
    reply += F("<BR>GW : ");
    reply += str;

    reply += F("<BR>Build : ");
    reply += BUILD;

    reply += F("<BR>Unit : ");
    reply += Settings.Unit;

    reply += F("<BR>STA MAC: ");
    uint8_t mac[] = {0,0,0,0,0,0};
    uint8_t* macread = WiFi.macAddress(mac);
    char macaddress[20];
    sprintf(macaddress,"%02x:%02x:%02x:%02x:%02x:%02x",macread[0],macread[1],macread[2],macread[3],macread[4],macread[5]);
    reply +=macaddress;

    reply += F("<BR>AP MAC: ");
    macread = WiFi.softAPmacAddress(mac);
    sprintf(macaddress,"%02x:%02x:%02x:%02x:%02x:%02x",macread[0],macread[1],macread[2],macread[3],macread[4],macread[5]);
    reply +=macaddress;

    reply += F("<BR>ESP Chip ID: ");
    reply += ESP.getChipId();

    reply += F("<BR>ESP Flash Size: ");
    reply += ESP.getFlashChipSize();

    reply += F("<BR>Free Mem: ");
    reply += ESP.getFreeHeap();

    reply += F("<BR><BR>Node List:<BR>");
    for (byte x=0; x<32;x++)
    {
    if(Nodes[x].ip[0] != 0)
      {
        if (x==Settings.Unit)
          reply += "<font color='blue'>";
        char url[80];
        sprintf(url,"<a href='http://%u.%u.%u.%u'>%u.%u.%u.%u</a>",Nodes[x].ip[0],Nodes[x].ip[1],Nodes[x].ip[2],Nodes[x].ip[3],Nodes[x].ip[0],Nodes[x].ip[1],Nodes[x].ip[2],Nodes[x].ip[3]);
        reply += "Unit ";
        reply += x;
        reply += " : ";
        reply += url;
        reply += " (age=";
        reply += Nodes[x].age;
        reply += ")<BR>";
        if (x==Settings.Unit)
          reply += "</font color>";
      }
    }
    
    reply += F("</form></body>");
    WebServer.send(200, "text/html", reply);
    printWebString = "";
    printToWeb=false;
    delay(100);
  }
  else
  {
    // have to disconnect from within the main loop
    // because the webconnection is still active at this point
    // disconnect would result into a crash/reboot...
    Serial.println(F("WIFI : Disconnecting..."));
    WebServer.send(200, "text/html", "OK");
    cmd_disconnect = true;
  }
}

//********************************************************************************
// Web Interface config page
//********************************************************************************
void handle_config() {
  char tmpstring[26];

  Serial.println(F("HTTP : Webconfig : "));

  String name = WebServer.arg("name");
  String ssid = WebServer.arg("ssid");
  String key = WebServer.arg("key");
  String controllerip = WebServer.arg("controllerip");
  String controllerport = WebServer.arg("controllerport");
  String protocol = WebServer.arg("protocol");
  String ip = WebServer.arg("ip");
  String espip = WebServer.arg("espip");
  String espgateway = WebServer.arg("espgateway");
  String espsubnet = WebServer.arg("espsubnet");
  String unit = WebServer.arg("unit");
  String apkey = WebServer.arg("apkey");
  String syslogip = WebServer.arg("syslogip");
  String udpport = WebServer.arg("udpport");

  if (ssid[0] != 0)
  {
    name.replace("+", " ");
    name.toCharArray(tmpstring, 25);
    strcpy(Settings.Name, tmpstring);
    ssid.toCharArray(tmpstring, 25);
    strcpy(Settings.WifiSSID, tmpstring);
    key.toCharArray(tmpstring, 25);
    strcpy(Settings.WifiKey, tmpstring);
    controllerip.toCharArray(tmpstring, 25);
    str2ip(tmpstring, Settings.Controller_IP);
    Settings.ControllerPort = controllerport.toInt();
    Settings.Protocol = protocol.toInt();
    Settings.IP_Octet = ip.toInt();
    espip.toCharArray(tmpstring, 25);
    str2ip(tmpstring, Settings.IP);
    espgateway.toCharArray(tmpstring, 25);
    str2ip(tmpstring, Settings.Gateway);
    espsubnet.toCharArray(tmpstring, 25);
    str2ip(tmpstring, Settings.Subnet);
    Settings.Unit = unit.toInt();
    apkey.toCharArray(tmpstring, 25);
    strcpy(Settings.WifiAPKey, tmpstring);
    syslogip.toCharArray(tmpstring, 25);
    str2ip(tmpstring, Settings.Syslog_IP);
    Settings.UDPPort = udpport.toInt();
    Save_Settings();
    LoadSettings();
  }

  String reply = F("<body><form>");
  reply += F("Name:<BR><input type='text' name='name' value='");
  Settings.Name[25]=0;
  reply += Settings.Name;
  reply += F("'><BR>SSID:<BR><input type='text' name='ssid' value='");
  reply += Settings.WifiSSID;
  reply += F("'><BR>WPA Key:<BR><input type='text' name='key' value='");
  reply += Settings.WifiKey;

  reply += F("'><BR>WPA AP Mode Key:<BR><input type='text' name='apkey' value='");
  reply += Settings.WifiAPKey;

  reply += F("'><BR>Unit nr:<BR><input type='text' name='unit' value='");
  reply += Settings.Unit;

  reply += F("'><BR>Controller IP:<BR><input type='text' name='controllerip' value='");
  char str[20];
  sprintf(str, "%u.%u.%u.%u", Settings.Controller_IP[0], Settings.Controller_IP[1], Settings.Controller_IP[2], Settings.Controller_IP[3]);
  reply += str;

  reply += F("'><BR>Controller Port:<BR><input type='text' name='controllerport' value='");
  reply += Settings.ControllerPort;

  reply +=F("'><BR>Protocol");
  byte choice = Settings.Protocol;
  String options[3];
  options[0]=F("");
  options[1]=F("Domoticz HTTP");
  options[2]=F("Domoticz MQTT");
  reply +=F("<BR><select name='protocol'>");
  for (byte x=0; x<3;x++)
    {
    reply +=F("<option value='");
    reply +=x;
    reply +="'";
    if (choice==x)
      reply +=" selected";
    reply +=">";
    reply +=options[x];
    reply+="</option>";
    }
  reply +=F("</select>");
  
  reply += F("<BR><BR>Optional<BR><BR>Fixed IP Octet:<BR><input type='text' name='ip' value='");
  reply += Settings.IP_Octet;

  reply += F("'><BR>ESP IP:<BR><input type='text' name='espip' value='");
  sprintf(str, "%u.%u.%u.%u", Settings.IP[0], Settings.IP[1], Settings.IP[2], Settings.IP[3]);
  reply += str;

  reply += F("'><BR>ESP GW:<BR><input type='text' name='espgateway' value='");
  sprintf(str, "%u.%u.%u.%u", Settings.Gateway[0], Settings.Gateway[1], Settings.Gateway[2], Settings.Gateway[3]);
  reply += str;

  reply += F("'><BR>ESP Subnet:<BR><input type='text' name='espsubnet' value='");
  sprintf(str, "%u.%u.%u.%u", Settings.Subnet[0], Settings.Subnet[1], Settings.Subnet[2], Settings.Subnet[3]);
  reply += str;

  reply += F("'><BR>Syslog IP:<BR><input type='text' name='syslogip' value='");
  str[0]=0;
  sprintf(str, "%u.%u.%u.%u", Settings.Syslog_IP[0], Settings.Syslog_IP[1], Settings.Syslog_IP[2], Settings.Syslog_IP[3]);
  reply += str;

  reply += F("'><BR>UDP port:<BR><input type='text' name='udpport' value='");
  reply += Settings.UDPPort;

  reply += F("'><BR><input type='submit' value='Submit'>");
  reply += F("</form></body>");
  WebServer.send(200, "text/html", reply);
  delay(1000);

}

//********************************************************************************
// Web Interface device page
//********************************************************************************
void handle_devices() {

  Serial.println(F("HTTP : Webdevices : "));

  String boardtype = WebServer.arg("boardtype");
  String sensordelay = WebServer.arg("delay");
  String dallas = WebServer.arg("dallas");
  String dht = WebServer.arg("dht");
  String dhttype = WebServer.arg("dhttype");
  String bmp = WebServer.arg("bmp");
  String lux = WebServer.arg("lux");
  String rfid = WebServer.arg("rfid");
  String analog = WebServer.arg("analog");
  String pulse1 = WebServer.arg("pulse1");
  String switch1 = WebServer.arg("switch1");

  if (sensordelay.toInt() != 0)
  {
    Settings.Delay = sensordelay.toInt();
    Settings.Dallas = dallas.toInt();
    Settings.DHT = dht.toInt();
    Settings.DHTType = dhttype.toInt();
    Settings.BMP = bmp.toInt();
    Settings.LUX = lux.toInt();
    Settings.RFID = rfid.toInt();
    Settings.Analog = analog.toInt();
    Settings.Pulse1 = pulse1.toInt();
    Settings.Switch1 = switch1.toInt();
    Save_Settings();
    #ifdef ESP_CONNEXIO

      struct NodoEventStruct TempEvent;
      ClearEvent(&TempEvent);
      for (byte x=1; x<50;x++)
        Eventlist_Write(x,&TempEvent,&TempEvent);
      EEPROM.commit();

      char cmd[80];
      sprintf(cmd,"eventlistwrite; boot %u; TimerSet 1,%u",Settings.Unit,Settings.Delay);
      ExecuteLine(cmd,VALUE_SOURCE_SERIAL);
      sprintf(cmd,"eventlistwrite; Timer 1; TimerSet 1,%u",Settings.Delay);
      ExecuteLine(cmd,VALUE_SOURCE_SERIAL);
      if (Settings.Dallas != 0)
      {
        eventAddTimer((char*)"DallasRead 1,1");
        eventAddVarSend(1, 1, Settings.Dallas);
      }
      if (Settings.DHT != 0)
      {
        eventAddTimer((char*)"DHTRead 2,2");
        eventAddVarSend(2, 2, Settings.DHT);
      }
      if (Settings.BMP != 0)
      {
        eventAddTimer((char*)"BMP085Read 4");
        eventAddVarSend(4, 3, Settings.BMP);
      }
      if (Settings.LUX != 0)
      {
        eventAddTimer((char*)"LuxRead 6");
        eventAddVarSend(6, 1, Settings.LUX);
      }
      if (Settings.Analog != 0)
      {
        eventAddTimer((char*)"VariableWiredAnalog 7");
        eventAddVarSend(7, 1, Settings.Analog);
      }
    #endif
  }

  String reply = F("<body><form>");
  reply += F("Delay:<BR><input type='text' name='delay' value='");
  reply += Settings.Delay;
  reply += F("'><BR>Dallas:<BR><input type='text' name='dallas' value='");
  reply += Settings.Dallas;
  reply += F("'><BR>DHT:<BR><input type='text' name='dht' value='");
  reply += Settings.DHT;
  reply += F("'><BR>DHT Type:<BR><input type='text' name='dhttype' value='");
  reply += Settings.DHTType;
  reply += F("'><BR>BMP:<BR><input type='text' name='bmp' value='");
  reply += Settings.BMP;
  reply += F("'><BR>LUX:<BR><input type='text' name='lux' value='");
  reply += Settings.LUX;
  reply += F("'><BR>RFID:<BR><input type='text' name='rfid' value='");
  reply += Settings.RFID;
  reply += F("'><BR>Analog:<BR><input type='text' name='analog' value='");
  reply += Settings.Analog;
  reply += F("'><BR>Pulse:<BR><input type='text' name='pulse1' value='");
  reply += Settings.Pulse1;
  reply += F("'><BR>Switch:<BR><input type='text' name='switch1' value='");
  reply += Settings.Switch1;

  reply += F("'><BR><input type='submit' value='Submit'>");
  reply += F("</form></body>");
  WebServer.send(200, "text/html", reply);
  delay(100);
  
}
#ifdef ESP_CONNEXIO
void eventAddVarSend(byte var, byte sensortype, int idx)
{
  char cmd[80];
  char strProtocol[5];
  if (Settings.Protocol == 1)
     strcpy(strProtocol,"HTTP");
  if (Settings.Protocol == 2)
     strcpy(strProtocol,"MQTT");
  sprintf(cmd,"eventlistwrite; Timer 1; VariableSend %u,%s,%u,%u",var,strProtocol,sensortype,idx);
  ExecuteLine(cmd,VALUE_SOURCE_SERIAL);
}

void eventAddTimer(char* event)
{
  char cmd[80];
  sprintf(cmd,"eventlistwrite; Timer 1; %s",event);
  ExecuteLine(cmd,VALUE_SOURCE_SERIAL);
}
#endif

//********************************************************************************
// Web Interface hardware page
//********************************************************************************
void handle_hardware() {

  Serial.println(F("HTTP : Hardware : "));

  String boardtype = WebServer.arg("boardtype");

  if (boardtype.length() != 0)
  {
    Settings.BoardType = boardtype.toInt();
    switch (Settings.BoardType)
      {
        case 0:
          Settings.Pin_i2c_sda     = 0;
          Settings.Pin_i2c_scl     = 2;
          Settings.Pin_wired_in_1  = 4;
          Settings.Pin_wired_in_2  = 5;
          Settings.Pin_wired_out_1 = 12;
          Settings.Pin_wired_out_2 = 13;
          break;
        case 1:
          Settings.Pin_i2c_sda     = 0;
          Settings.Pin_i2c_scl     = 2;
          Settings.Pin_wired_in_1  = -1;
          Settings.Pin_wired_in_2  = -1;
          Settings.Pin_wired_out_1 = -1;
          Settings.Pin_wired_out_2 = -1;
          break;
        case 2:
          Settings.Pin_i2c_sda     = -1;
          Settings.Pin_i2c_scl     = -1;
          Settings.Pin_wired_in_1  = 0;
          Settings.Pin_wired_in_2  = -1;
          Settings.Pin_wired_out_1 = 2;
          Settings.Pin_wired_out_2 = -1;
          break;
        case 3:
          Settings.Pin_i2c_sda     = -1;
          Settings.Pin_i2c_scl     = -1;
          Settings.Pin_wired_in_1  = 0;
          Settings.Pin_wired_in_2  = 2;
          Settings.Pin_wired_out_1 = -1;
          Settings.Pin_wired_out_2 = -1;
          break;
        case 4:
          Settings.Pin_i2c_sda     = -1;
          Settings.Pin_i2c_scl     = -1;
          Settings.Pin_wired_in_1  = -1;
          Settings.Pin_wired_in_2  = -1;
          Settings.Pin_wired_out_1 = 0;
          Settings.Pin_wired_out_2 = 2;
          break;

      }
    Save_Settings();
  }

  String reply = F("<body><form>");

  reply +="Board Type:";
  byte choice = Settings.BoardType;
  String options[5];
  options[0]=F("ESP-07/12");
  options[1]=F("ESP-01 I2C");
  options[2]=F("ESP-01 In/Out");
  options[3]=F("ESP-01 2 x In");
  options[4]=F("ESP-01 2 x Out");
  reply +=F("<BR><select name='boardtype'>");
  for (byte x=0; x<5;x++)
    {
    reply +=F("<option value='");
    reply +=x;
    reply +="'";
    if (choice==x)
      reply +=" selected";
    reply +=">";
    reply +=options[x];
    reply+="</option>";
    }
  reply +=F("</select>");



    switch (Settings.BoardType)
      {
        case 0:
          reply +=F("<BR>SDA: ");
          reply += Settings.Pin_i2c_sda;
          reply +=F("<BR>SCL: ");
          reply += Settings.Pin_i2c_scl;
          reply +=F("<BR>Input 1: ");
          reply += Settings.Pin_wired_in_1;
          reply +=F("<BR>Input 2: ");
          reply += Settings.Pin_wired_in_2;
          reply +=F("<BR>Output 1: ");
          reply += Settings.Pin_wired_out_1;
          reply +=F("<BR>Output 2: ");
          reply += Settings.Pin_wired_out_2;
          break;
        case 1:
          reply +=F("<BR>SDA: ");
          reply += Settings.Pin_i2c_sda;
          reply +=F("<BR>SCL: ");
          reply += Settings.Pin_i2c_scl;
          break;
        case 2:
          reply +=F("<BR>Input 1: ");
          reply += Settings.Pin_wired_in_1;
          reply +=F("<BR>Output 1: ");
          reply += Settings.Pin_wired_out_1;
          break;
        case 3:
          reply +=F("<BR>Input 1: ");
          reply += Settings.Pin_wired_in_1;
          reply +=F("<BR>Input 2: ");
          reply += Settings.Pin_wired_in_2;
          break;
        case 4:
          reply +=F("<BR>Output 1: ");
          reply += Settings.Pin_wired_out_1;
          reply +=F("<BR>Output 2: ");
          reply += Settings.Pin_wired_out_2;
          break;
      }
  
  reply += F("<BR><input type='submit' value='Submit'>");
  reply += F("</form></body>");
  WebServer.send(200, "text/html", reply);
  delay(100);
}

//********************************************************************************
// Nodo proof of concept. send json query as nodo event on I2C to mega
// Compatible with Nodo 3.8 only, tested on R818
// set used variables to global on the Mega.
//********************************************************************************

#define NODO_VERSION_MAJOR   3
#define TARGET_NODO          5

struct TransmissionStruct
{
  byte Type;
  byte Command;
  byte Par1;
  byte Dummy;
  unsigned long Par2;
  byte P1;
  byte P2;
  byte SourceUnit;
  byte DestinationUnit;
  byte Flags;
  byte Checksum;
};

void handle_json() {
  Serial.print(F("HTTP : Web json : idx: "));
  String idx = WebServer.arg("idx");
  String svalue = WebServer.arg("svalue");
  Serial.print(idx);
  Serial.print(" svalue: ");
  Serial.println(svalue);
  char c_idx[10];
  c_idx[0] = 0;
  idx.toCharArray(c_idx, 9);
  char c_svalue[40];
  c_svalue[0] = 0;
  svalue.toCharArray(c_svalue, 39);

  struct TransmissionStruct event;
  event.Type = 1;
  event.Command = 4;
  event.Par1 = str2int(c_idx);
  event.Par2 = float2ul(atof(c_svalue));
  event.P1 = 0;
  event.P2 = 0;
  event.SourceUnit = 1;
  event.DestinationUnit = 0;
  event.Flags = 0;
  event.Checksum = 0;

  // due to padding of structs in memory on this MCU, we need to shift some bytes
  byte data[13];
  memcpy((byte*)&data, (byte*)&event, 3);
  memcpy((byte*)&data + 3, (byte*)&event + 4, 10);

  // calculate xor checksum
  byte NewChecksum = NODO_VERSION_MAJOR;
  for (byte x = 0; x < sizeof(data); x++)
    NewChecksum ^= data[x];
  data[12] = NewChecksum;

  // Send data to Nodo through I2C bus
  // Currently the target Nodo nr is fixed
  // I2C implementation is still incomplete, scanning does not work, slave mode not supported yet...
  Wire.beginTransmission(TARGET_NODO);
  for (byte x = 0; x < sizeof(data); x++)
    Wire.write(data[x]);
  Wire.endTransmission();

  WebServer.send(200, "text/html", "OK");
  delay(100);

}

#ifdef ESP_CONNEXIO
//********************************************************************************
// Web Interface eventlist page
//********************************************************************************
void handle_eventlist() {
  Serial.print(F("HTTP : Eventlist request : "));

  char *TempString=(char*)malloc(80);
  String reply =F("Eventlist<BR><BR>");

  if (WebServer.args() ==1)
    {

      struct NodoEventStruct TempEvent;
      ClearEvent(&TempEvent);
      for (byte x=1; x<50;x++)
        Eventlist_Write(x,&TempEvent,&TempEvent);
      EEPROM.commit();

      String eventlist = WebServer.arg("eventlist");
      eventlist.replace("%0D%0A", "\n");
      int NewLineIndex = eventlist.indexOf('\n');
      byte limit=0;
      byte messagecode=0;
      while (NewLineIndex > 0 && limit < 10)
        {
          limit++;
          String line=eventlist.substring(0,NewLineIndex);
          line.replace("%3B", ";");
          line.replace("%2C", ",");
          line.replace("+", " ");
          //int SemiColonIndex = line.indexOf(';');
          //line = line.substring(SemiColonIndex+1);
          String strCommand = F("eventlistwrite;");
          strCommand += line;
          Serial.println(strCommand);
          strCommand.toCharArray(TempString,79);
          messagecode=ExecuteLine(TempString,VALUE_SOURCE_SERIAL);
          if (messagecode > 0)
          {
            reply += TempString;
            reply += " : ";
            reply += MessageText_tabel[messagecode];
            reply += "<BR>";
          }
          eventlist = eventlist.substring(NewLineIndex+1);
          NewLineIndex = eventlist.indexOf('\n');
        }
    }

    reply += F("<body><form method='post'>");
    reply += F("<textarea name='eventlist' rows='15' cols='80' wrap='on'>");
    byte x=1;
    while(EventlistEntry2str(x++,0,TempString,false))
      if(TempString[0]!=0)
        {
          reply += TempString;
          reply += '\n';
        }

    reply += F("</textarea>");
  
    reply += F("<BR><input type='submit' value='Submit'>");
    reply += F("</form></body>");

    WebServer.send(200, "text/html", reply);

    free(TempString);
}
#endif

//********************************************************************************
// Web Interface log page
//********************************************************************************
void handle_log() {
  Serial.println(F("HTTP : Log request : "));

  char *TempString=(char*)malloc(80);

  String reply = F("<script language='JavaScript'>function RefreshMe(){window.location = window.location}setTimeout('RefreshMe()', 3000);</script>Log<BR><BR>");

  byte counter=logcount;
  do
    {
      counter++;
      if (counter > 9)
        counter=0;
      if (Logging[counter].timeStamp > 0)
        {
          reply += Logging[counter].timeStamp;
          reply +=" : ";
          reply += Logging[counter].Message;
          reply +="<BR>";
        }
    }  while (counter != logcount);

  WebServer.send(200, "text/html", reply);
  free(TempString);
}

//********************************************************************************
// Web Interface debug page
//********************************************************************************
void handle_debug() {
  Serial.print(F("HTTP : Webrequest : "));
  String webrequest = WebServer.arg("cmd");
  webrequest.replace("%3B", ";");
  webrequest.replace("%2C", ",");
  webrequest.replace("+", " ");
  Serial.println(webrequest);
  char command[80];
  command[0] = 0;
  webrequest.toCharArray(command, 79);

  String reply = "";
  reply += F("<body><form>Command:<BR>");
  reply += F("<input type='text' name='cmd' value='");
  reply += webrequest;
  reply += F("'><BR><input type='submit' value='Submit'>");

  printToWeb=true;
  printWebString = "<BR>";
  #ifdef ESP_CONNEXIO
    ExecuteLine(command,VALUE_SOURCE_SERIAL);
  #endif
  #ifdef ESP_EASY
    ExecuteCommand(command);
  #endif
  reply += printWebString;
  reply += F("</form></body>");
  WebServer.send(200, "text/html", reply);
  printWebString = "";
  printToWeb=false;
}
