#!/usr/bin/ruby
require 'serialport'
require 'rubygems'


sp = SerialPort.new "/dev/ttyAMA0", 19200
sp.parity=SerialPort::EVEN
sp.flush
while 1 do
        puts sp.getc.unpack('H*')

end

