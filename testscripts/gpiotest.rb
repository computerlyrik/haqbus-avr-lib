#!/usr/bin/ruby
require 'serialport'
require 'rubygems'
require 'wiringpi'



$io = WiringPi::GPIO.new
$io.mode(0,OUTPUT)

$serial = WiringPi::Serial.new("/dev/ttyAMA0",19200) 

def send_byte(byte,parity)
#  puts "sending byte "+byte.to_s(2)
  if  (byte.to_s(2).count('1').even?) ^ (parity == 1)
    $serial.serialParity(0)
#    puts "parity even"
  else 
    $serial.serialParity(1)
#    puts "parity odd"
  end
  $serial.serialPutchar byte
end

def rec_bytes(num,parity)
#  $sp.parity=parity
  num.times {
    puts $serial.serialGetchar.unpack('H*')
  }
end

bytes  = [0,3,0x66,0x6f,0x6f,0,0x80]
while 1 do

#puts "---"

  $io.write(0,HIGH)
#  puts "##address"
  send_byte(0,1)
  send_byte(0,1)
#  puts "##block"
  bytes.each do |b|
#	puts "writing "+b.to_s(16)
  	send_byte(b,0)
  end
#puts "---------------------------"

 # $io.write(0,LOW)
 # rec_bytes(2,1)
 # rec_bytes(7,0)
sleep 0.01
end



