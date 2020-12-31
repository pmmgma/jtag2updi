import serial
import sys
import crcmod
import time
import threading
import msvcrt

CMND_SIGN_OFF				= b'\x00'
CMND_GET_SIGN_ON			= b'\x01'
CMND_SET_PARAMETER			= b'\x02'
CMND_GET_PARAMETER			= b'\x03'
CMND_WRITE_MEMORY			= b'\x04'
CMND_READ_MEMORY			= b'\x05'
CMND_GO						= b'\x08'
CMND_RESET					= b'\x0b'
CMND_SET_DEVICE_DESCRIPTOR	= b'\x0c'
CMND_GET_SYNC				= b'\x0f'
CMND_ENTER_PROGMODE			= b'\x14'
CMND_LEAVE_PROGMODE			= b'\x15'
CMND_XMEGA_ERASE			= b'\x34'
CMND_TERMINAL    			= b'\x70'



PARAM_HW_VER				= b'\x01'
PARAM_FW_VER				= b'\x02'
PARAM_EMU_MODE				= b'\x03'
PARAM_BAUD_RATE				= b'\x05'
PARAM_VTARGET				= b'\x06'


baud_2400	= b'\x01'
baud_4800   = b'\x02'
baud_9600   = b'\x03'
baud_19200	= b'\x04'
baud_38400  = b'\x05'
baud_57600  = b'\x06'
baud_115200 = b'\x07'
baud_14400  = b'\x08'

# Success
RSP_OK						= b'\x80'
RSP_PARAMETER				= b'\x81'
RSP_MEMORY					= b'\x82'
RSP_SIGN_ON					= b'\x86'
# Error                       
RSP_FAILED					= b'\xA0'
RSP_ILLEGAL_PARAMETER		= b'\xA1'
RSP_ILLEGAL_MEMORY_TYPE		= b'\xA2'
RSP_ILLEGAL_MEMORY_RANGE	= b'\xA3'
RSP_ILLEGAL_MCU_STATE		= b'\xA5'
RSP_ILLEGAL_VALUE			= b'\xA6'
RSP_ILLEGAL_BREAKPOINT		= b'\xA8'
RSP_ILLEGAL_JTAG_ID			= b'\xA9'
RSP_ILLEGAL_COMMAND			= b'\xAA'
RSP_NO_TARGET_POWER			= b'\xAB'
RSP_DEBUGWIRE_SYNC_FAILED	= b'\xAC'
RSP_ILLEGAL_POWER_STATE		= b'\xAD'

crc16 = crcmod.mkCrcFun(0x11021, 0xffff, True, 0x0000)

packet_num=0
DEBUG=0

def sendpacket(ser,body):
    global DEBUG
    global packet_num
    packet_num+=1
    pck=b'\x1b'+packet_num.to_bytes(2, 'little')+len(body).to_bytes(4, 'little')+b'\x0e'+body
    pck=pck+crc16(pck).to_bytes(2, 'little')
    if DEBUG: print(pck)
    ser.write(pck)
    return packet_num


def processanswer(ser,num):
    global DEBUG
    while True:
        start=ser.read(1)
        if DEBUG: print(start)
        if not(start): return None
        if start!=b'\x1b': continue
        crcc=crc16(start)
        
        numb=ser.read(2)
        if DEBUG: print(numb)
        if not(numb): return None
        crcc=crc16(numb,crcc)
        numr=int.from_bytes(numb, byteorder='little')

        sizeb=ser.read(4)
        if DEBUG: print(sizeb)
        if not(sizeb): return None
        crcc=crc16(sizeb,crcc)
        size=int.from_bytes(sizeb, byteorder='little')
        
        token=ser.read(1)
        if DEBUG: print(token)
        if not(token): return None
        if token!=b'\x0e': continue
        crcc=crc16(token,crcc)
        
       
        body=ser.read(size)
        if DEBUG: print(body)
        if not(body): return None
        crcc=crc16(body,crcc)

        crc=ser.read(2)
        if DEBUG: print(crc,crcc.to_bytes(2, 'little'))
        if not(crc): return None
        if crc!=crcc.to_bytes(2, 'little'): continue
        
        if DEBUG: print(num,numr)
        if num!=numr: continue
        
        return body
        
        
def SendRetry(ser, body, ex_ans):
    retry=3
    while True:
        num=sendpacket(ser,body)
        ans=processanswer(ser,num)
        if not(ans) or ans[0:1]!=ex_ans: 
            time.sleep(0.3)
            retry-=1
            if retry==0:
                retry=3
            else:
                print("Retry...")
            continue
            
            
        break
       
    return ans

ser = serial.Serial()
ser.port =sys.argv[1]
ser.baudrate = 19200
ser.timeout = 0.1
ser.setDTR(False)
ser.open()

    
print("Connecting at 19200 to "+ser.name+".")

print("Sign on...")
x=SendRetry(ser, CMND_GET_SIGN_ON, RSP_SIGN_ON );


print("Reset UPDI interface...")
x=SendRetry(ser, CMND_SET_DEVICE_DESCRIPTOR, RSP_OK );

   
print("Enable debug terminal...")
x=SendRetry(ser, CMND_TERMINAL, RSP_OK );
    
print("Terminal running...\n",flush=True)

runnning=1

def thread_function(name):
    global runnning
    while True:
        ch=msvcrt.getch()
        if ch: 
            if ch==b'\x03': break
            ser.write(ch)
            
    runnning=0
    

x = threading.Thread(target=thread_function, args=(1,))
x.start()

while runnning:
    ch=ser.read(1);
    if ch: print(ch.decode("utf-8",errors='replace') ,flush=True, end="")
        
print("\n\nSign off...")
x=SendRetry(ser, CMND_SIGN_OFF, RSP_OK );

ser.close()             # close port