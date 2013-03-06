#!/usr/bin/ruby
require 'serialport'
require 'rubygems'
require 'wiringpi'



$io = WiringPi::GPIO.new
$io.mode(0,OUTPUT)

$serial = WiringPi::Serial.new("/dev/ttyAMA0",9600) 


bytes  = [0,3,0x66,0x6f,0x6f,0,0x80]
$serial.serialParity(0)
while 1 do
  $io.write(0,HIGH)

  $serial.serialPut9char( 0b10100001,1)
  $serial.serialPut9char( 0b00000000,0)

#  $serial.serialPut9char( 0b01000010,1)
#  $serial.serialPut9char(0b10100001,1)
#  $serial.serialPut9char(0,0)
#  $serial.serialPut9char(0,0)
#  $serial.serialPut9char(0,1)
#  bytes.each do |b|
# 	$serial.serialPut9char(b,0)
#  end
 
 # $io.write(0,LOW)
 # rec_bytes(2,1)
 # rec_bytes(7,0)
sleep 0.01
end



