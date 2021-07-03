from flask import Flask, render_template, url_for, request, redirect
import sys
import os
current = os.path.dirname(os.path.realpath(__file__))
parent = os.path.dirname(current)
sys.path.append(parent)
from BE.log_sender import *
import pastebin
import json
import xmltodict

app = Flask(__name__)

def latestLogs():
   try:
        api = get_pastebin_api()
        print(api)
   except Exception as e:
        print(e)
#    print(thisRes['paste']['paste_key'])
   #Get all pastes
   thisRes = xmltodict.parse(api.list_pastes())
   print(thisRes)
   logsIn = []
   for paste in thisRes:
       logsIn.append(json.loads(api.raw_pastes(paste['paste']['paste_key'])))
       
   return logsIn
#    return(api.raw_pastes())
   
        
    


@app.route('/')
def hello():
    
    logs = latestLogs()
    return render_template('logs.html', logs = logs)