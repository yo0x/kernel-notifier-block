'''
Small front end application that shows the latest paste logs from pastebin.com
The logs are shown on the webpage's template.
'''
from flask import Flask, render_template, url_for, request, redirect
import sys
import os
import time
from xml.etree import ElementTree
current = os.path.dirname(os.path.realpath(__file__))
parent = os.path.dirname(current)
sys.path.append(parent)
from BE.log_sender import *
import pastebin
import json
import xmltodict

app = Flask(__name__)

def latestLogs():
   '''
   Get latest paste logs from pastebin.com
   Show them on the webpage's template
   ''' 
    
   try:
        api = get_pastebin_api()
        print("API ".format(api))
   except Exception as e:
        print("DEBB NONO \n")
        print(e)
   #Get all pastes
   try:
       print(api.list_pastes())
       raw_pastes = []
       new_dic = {}
       thisRes = api.list_pastes()
       xml = ElementTree.fromstring('<list>' + thisRes + '</list>')
       result = xml.findall(".//paste/paste_key")
       for i in result:
           raw_pastes.append(json.loads(api.raw_pastes(i.text)))
           time.sleep(1)
           print(i.text)
       for a in raw_pastes:
           print(a)
       for soloPaste in raw_pastes:
           for new_k in soloPaste:
               new_dic[new_k] = soloPaste[new_k]
               
               
   except Exception as e:
       print(e)
   return(new_dic)
   
        
    


@app.route('/')
def hello():
    dic_json = latestLogs()
    for key in dic_json:
        print(key)
        print(dic_json[key])
    #return "Hello{}".format(logs)
    return render_template('logs.html', logs =dic_json )