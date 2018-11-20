from picamera.array import PiRGBArray
from picamera import PiCamera
import time
import cv2
import sys
import imutils
from fractions import Fraction
import base64
import requests
import json
import random
import os

# Get user supplied values
cascPath = './haarcascade_frontalface_default.xml'

# Create the haar cascade
faceCascade = cv2.CascadeClassifier(cascPath)

# initialize the camera and grab a reference to the raw camera capture
camera = PiCamera()
camera.resolution = (160, 120)
camera.framerate = 32
rawCapture = PiRGBArray(camera, size=(160, 120))

# allow the camera to warmup
time.sleep(0.1)
lastTime = time.time()*1000.0
# capture frames from the camera
for frame in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
	# grab the raw NumPy array representing the image, then initialize the timestamp
	# and occupied/unoccupied text
    image = frame.array
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    
    # Detect faces in the image
    faces = faceCascade.detectMultiScale(
    gray,
    scaleFactor=1.1,
    minNeighbors=5,
    minSize=(30, 30),
    flags = cv2.cv.CV_HAAR_SCALE_IMAGE
    )
    print time.time()*1000.0-lastTime," Found {0} faces!".format(len(faces))
    lastTime = time.time()*1000.0

    # Draw a rectangle around the faces
    for (x, y, w, h) in faces:
        cv2.circle(image, (x+w/2, y+h/2), int((w+h)/3), (255, 255, 255), 1)
    # show the frame
    cv2.imshow("Frame", image)
    key = cv2.waitKey(1) & 0xFF
    if len(faces) == 1:
        print("Taking image...")
	camera.capture("foo.jpg")
	os.system('espeak "Human face detected"')
	inputImage= "./foo.jpg"
	del camera
	break 
	# clear the stream in preparation for the next frame
    rawCapture.truncate(0)
    
	# if the `q` key was pressed, break from the loop
    if key == ord("q"):
        del camera
        exit()
        
KAIROS = "api.kairos"
KairosGallery = 'MyFace'
KairosConfig = './kairos_config.json'

def trainKairos(image, name):
    global KairosGallery
    headers = {
        'app_id': 'your-app-id',
        'app_key': 'your-app-key'
    }
    data = {
        'image': base64.b64encode(image),
        'gallery_name': KairosGallery,
        'subject_id': name
    }
    r = requests.post('http://api.kairos.com/enroll', headers=headers, data=json.dumps(data))
    print(r.text)
    return(None)

class Recognize():
    def __init__(self, API, config_file):
        self.api = API
        self.config = config_file

    #def recognize(self, image_path):
    #    return self.__recognizeKairos(image_path)
    
    def recognizeKairos(self, image):
        with open(image, "rb") as image_file:
            encoded_string = base64.b64encode(image_file.read())
        with open(self.config, "rb") as config_file:
            config = json.loads(config_file.read())
        data = {
            "image": encoded_string,
            "gallery_name": config["gallery_name"]
        }

        headers = {
            "Content-Type": "application/json",
            "app_id": config["app_id"],
            "app_key": config["app_key"]
        }
        try:
            r = requests.post("https://api.kairos.com/recognize", headers=headers, data=json.dumps(data))
            data = r.json()
            print data
            # print json.dumps(data, indent=4)
            faces = []
            if "images" in data:
                for obj in data["images"]:
                    if obj["transaction"]["status"] == "success":
                        face_obj = {}
                        face_obj["person"] = obj["transaction"]["subject_id"].decode("utf_8")
                        #face_obj["faceid"] = obj["candidates"][0]["face_id"].decode("utf_8")
                        face_obj["confidence"] = obj["transaction"]["confidence"]
                        faces.append(face_obj)
                    elif obj["transaction"]["status"] == "failure":
                        face_obj = {}
                        face_obj["person"] = "unidentified"
                        face_obj["confidence"] = 0
                        faces.append(face_obj)
                    else:
                        print "its in last loop"
                return faces
        except requests.exceptions.RequestException as exception:
            print exception
        return None
    

if __name__ == "__main__":
    r = Recognize(KAIROS, "kairos_config.json")
    x = r.recognizeKairos(inputImage)
    
    #print x
    #print x["person"]
    #print x[0]["person"]
    string1 = x[0]["person"]
    #print string1
    os.system('espeak "Hello...""{}"'.format(string1))
    if x[0]["person"] == "unidentified":
        os.system('espeak "Please enter your name to Register"')
        nameToRegister = raw_input("Please enter your name to Register :")
        binaryData = open(inputImage, 'rb').read()
        print('Enrolling to Kairos')
        trainKairos(binaryData, nameToRegister)
        print "You are now Registered as :", nameToRegister
        os.system('espeak "Hello...""{}"'.format(nameToRegister))
        exit()
