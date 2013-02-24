haqbus-avr-lib
==============

Library for using package based haqbus


# Desired Periphals

Should emit following json format
```
{ address: "string",
  description: "string",
  inputs:
    { name: "mySignalName1",
      type: "int",
      min: 0,
      max: 255
    }
  outputs:
    { name: "mySignalName1,
      type: "int"
    }
}




## Lightning

### Single LED
- Inputs: brightness (int 0-255)
- Outputs: brightness

### MultiChannel LED
- Inputs: Channel1...Channeln (Or Led_R, Led_G...) (int 0-255)
- Outpts: see above

## RFID
- Inputs: Open/Close Door (bool)
- Outputs: RFID Key (String64)

## Switcher Array
- Inputs: On/Off (bool) (n times)
- Outputs: see above
