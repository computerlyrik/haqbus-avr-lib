#!/usr/bin/ruby
require 'serialport'
require 'rubygems'

#TODO add gpio config; test
$sp = SerialPort.new "/dev/ttyAMA0", 19200
$sp.flush


def send_byte(byte,parity)
  puts "sending byte "+byte.to_s(2)
  if  (byte.to_s(2).count('1').even?) ^ (parity == 1)
    $sp.parity=SerialPort::EVEN
    puts "parity even"
  else
    $sp.parity=SerialPort::ODD
    puts "parity odd"
  end
  $sp.write byte
end

bytes  = [0,3,0x66,0x6f,0x6f,0,0x80]
while 1 do
  puts "##address"
  send_byte(0,1)
  send_byte(0,1)
  puts "##block"
  bytes.each do |b|
	puts "writing "+b.to_s(16)
	send_byte(b,0)
  end
puts "---------------------------"
end

