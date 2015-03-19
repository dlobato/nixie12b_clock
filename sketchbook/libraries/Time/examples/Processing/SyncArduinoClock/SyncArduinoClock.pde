/**
 * SyncArduinoClock. 
 *
 * portIndex must be set to the port connected to the Arduino
 * 
 * The current time is sent in response to request message from Arduino 
 * or by clicking the display window 
 *
 * The time message is 11 ASCII text characters; a header (the letter 'T')
 * followed by the ten digit system time (unix time)
 */
 

import processing.serial.*;
import java.util.Date;
import java.util.Calendar;
import java.util.GregorianCalendar;
import controlP5.*;


public static final String TIME_HEADER = "T"; //header for arduino serial time message 
public static final char TIME_REQUEST = 7;  // ASCII bell character 
public static final char LF = 10;     // ASCII linefeed
public static final char CR = 13;     // ASCII linefeed
short portIndex = 0;  // select the com port, 0 is the first port
Serial myPort;     // Create object from Serial class

ControlP5 controlP5;
DropdownList l;
Button b;

void setup() {  
  size(140,160);
  controlP5 = new ControlP5(this);
  l = controlP5.addDropdownList("serialList")
         .setPosition(10, 50)
         .setSize(120, 100)
         .setItemHeight(15)
         .setBarHeight(15)
         .setColorBackground(color(255, 128))
         .setColorActive(color(0))
         .setColorForeground(color(255, 100,0))
         ;
  l.captionLabel().toUpperCase(true);
  l.captionLabel().set("Puerto serie");
  l.captionLabel().setColor(0xffff0000);
  l.captionLabel().style().marginTop = 3;
  l.valueLabel().style().marginTop = 3;
  
  for(int i=0;i<Serial.list().length;i++) {
    ListBoxItem lbi = l.addItem(Serial.list()[i],i);
    lbi.setColorBackground(0xffff0000);
  }
  
  b = controlP5.addButton("sync")
               .setPosition(10,10)
               .setSize(120,20)
               ;
  b.captionLabel().set("Sincronizar hora");
}

void draw()
{
  background(128);
}

public void controlEvent(ControlEvent theEvent) {
  if (theEvent.isGroup()) {//dropdownlist
    // check if the Event was triggered from a ControlGroup
    //println("event from group : "+theEvent.getGroup().getValue()+" from "+theEvent.getGroup());
    portIndex = (short)theEvent.getGroup().getValue();
    if (myPort != null) 
      myPort.stop();
    println("Conectando a -> " + Serial.list()[portIndex]);
    myPort = new Serial(this,Serial.list()[portIndex], 9600);
  } 
  else if (theEvent.isController()) {//button
    //println("event from controller : "+theEvent.getController().getValue()+" from "+theEvent.getController());
    long t = getTimeNow();
    println("Enviando nuevo tiempo: " + t);
    if (myPort != null)
      sendTimeMessage(TIME_HEADER,getTimeNow());
  }
}


void sendTimeMessage(String header, long time) {  
  String timeStr = String.valueOf(time);  
  myPort.write(header);  // send header and time to arduino
  myPort.write(timeStr); 
  myPort.write('\n');  
}

long getTimeNow(){
  // java time is in ms, we want secs    
  Date d = new Date();
  Calendar cal = new GregorianCalendar();
  long current = d.getTime()/1000;
  long timezone = cal.get(cal.ZONE_OFFSET)/1000;
  long daylight = cal.get(cal.DST_OFFSET)/1000;
  return current + timezone + daylight; 
}
