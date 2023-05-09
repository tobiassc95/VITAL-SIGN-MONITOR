#from pydub import AudioSegment
#from pydub.playback import play
import io
import os
import glob

print(os.listdir())

all_mp3 = []
os.chdir("MP3Files")
for file in glob.glob("*.mp3"):
    all_mp3.append(file)

os.chdir('..') #to go back (previous folder).

SECTORS_NEEDED = {}
TOTAL_SECTORS_NEEDED = 0
SECTOR_INDEX = {}
for filename in all_mp3:
    data = open("MP3Files/"+filename, 'rb').read()
    print("\nTotal number of bytes of "+filename+": "+str(len(data)))
    SECTORS_NEEDED[filename] = len(data)//(4*1024) + 1
    print("Number of sectors of "+filename+": "+str(SECTORS_NEEDED[filename]))
    SECTOR_INDEX[filename] = TOTAL_SECTORS_NEEDED
    TOTAL_SECTORS_NEEDED += SECTORS_NEEDED[filename]
    print("Sector index of "+filename+": "+str(SECTOR_INDEX[filename]))

print(f"\nTotal sectors needed: {TOTAL_SECTORS_NEEDED} ({(TOTAL_SECTORS_NEEDED*4096)} Bytes).")
print(f"Total memory flash occupied: {(TOTAL_SECTORS_NEEDED*4*1024)*100/((512*2)*1024)}%")

for filename in all_mp3:
    data = open("MP3Files/"+filename, 'rb').read()
    
    f = open("hexFiles/"+filename[:-4]+".bin",'w') #'filename[:-4]' is to remove the '.mp3'
    for el in data:
        f.write(str(hex(el))[2:]) #'str(hex(el))[2:]' is to remove '0x' from hex number.
    f.close()

    arr_str = "uint8_t "+filename[:-4]+"_array["+filename[:-4].upper()+"_ARRAY_SIZE] = { \n\t"
    #arr_str = "uint8_t "+filename[:-4]+"_array[] = { \n"

    columns = 30
    for i,el in enumerate(data):
        hex_num = "0x"+format(el, '02x')
        arr_str += hex_num + ","
        if((i-(columns-1)) % columns == 0):
            arr_str += "\n\t"

    if(arr_str[-1] == ","):
        arr_str = arr_str[:-1] #To remove the ',' in the end of the string.
    arr_str += "\n};"

            
    f = open("hFiles/"+filename[:-4]+"_array.h",'w')
    f.write("#ifndef "+filename[:-4].upper()+"_ARRAY_H\n")
    f.write("#define "+filename[:-4].upper()+"_ARRAY_H\n")
    f.write("\n#include <stdint.h>\n\n")
    f.write("#define "+filename[:-4].upper()+"_ARRAY_SIZE "+str(len(data))+"\n\n")
    #f.write("//sizeof("+filename[:-4]+"_array) = "+str(len(data))+"\n")

    f.write(arr_str)
    f.write("\n\n#endif //"+filename[:-4].upper()+"_ARRAY_H")
    f.close()