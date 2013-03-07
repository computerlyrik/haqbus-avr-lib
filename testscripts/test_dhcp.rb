#!/usr/bin/ruby
require 'serialport'
require 'rubygems'
require 'wiringpi'



$io = WiringPi::GPIO.new
$io.mode(0,OUTPUT)

$serial = WiringPi::Serial.new("/dev/ttyAMA0",19200) 
$serial.serialParity(0)

def rec_bytes(num,parity)
  num.times {
    puts $serial.serialGetchar.to_s(16) #.unpack('H*')
  }
end


while 1 do
sleep 0.01 
  $io.write(0,LOW)
sleep 0.01 

rec_bytes(2,1)
rec_bytes(2,0)
id = $serial.serialGetchar
puts "id: "+id.to_s(16) 
rec_bytes(2,0)
puts "sending"
sleep 0.01 
  $io.write(0,HIGH)
sleep 0.01 


  $serial.serialPut9char(0,1)
  $serial.serialPut9char(0,1)
bytes  = [0,3,id,0x6f,0x5,0x3,0x80]
  bytes.each do |b|
 	$serial.serialPut9char(b,0)
  end
puts "sent"
sleep 0.01 
  $io.write(0,LOW)
sleep 0.01 
  rec_bytes(2,1)
  rec_bytes(7,0)


sleep 0.01 
  $io.write(0,HIGH)
sleep 0.01 
  $serial.serialPut9char(0x6f,1)
  $serial.serialPut9char(0x5,1)
bytes  = [0,4,5,0x6f,0x5,0x3,0, 0x80]
  bytes.each do |b|
 	$serial.serialPut9char(b,0)
  end
puts "sent"
puts '----'
end



