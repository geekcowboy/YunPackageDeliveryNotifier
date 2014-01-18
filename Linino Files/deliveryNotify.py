#######################################################
## File:    deliveryNotify.py
## Author:  Mike Parks  http://greenshoegarage.com
## Version: 1.0
## Date Last Revised:  18JAN2014
## Purpose:  Creates and sends email
##
########################################################

# os for accessing filesystem to get to mnt/sda1/webcam photos
import os

# SMTP Library to send email securely
import smtplib

# Needed to build an email message
from email.mime.image import MIMEImage
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText

# Key components of an email message
from_addr = 'your-email-address@gmail.com'
PASSWORD = 'yourpassword'
to_addr = 'destination-email-address@gmail.com'
text = 'We thought you would like to know that your package has arrived!'

# Create the Multipart MIME
msg = MIMEMultipart()
msg['Subject'] = 'Package Delivered.'
msg['From'] = from_addr
msg['To'] = to_addr
msg.preamble = 'Your package has arrived.'
msg.attach(MIMEText(text, 'plain'))

# Find and attach the latest picture in the /mnt/sda1/webcam folder
fileDir = "/mnt/sda1/webcam"   
os.chdir(fileDir)
filelist = os.listdir(os.getcwd())
filelist = filter(lambda x: not os.path.isdir(x), filelist)
attachment = max(filelist, key=lambda x: os.stat(x).st_mtime)

# Open file and attach
fp = open(attachment, 'rb')
img = MIMEImage(fp.read(), name = "image.jpg")
fp.close()
msg.attach(img)

# Send the email message
server = smtplib.SMTP('smtp.gmail.com:587')
server.ehlo_or_helo_if_needed()
server.starttls()
server.ehlo_or_helo_if_needed()
server.login(from_addr,PASSWORD)
server.sendmail(from_addr, to_addr, msg.as_string())
server.quit()

# Notify user that email has been sent
print 'Email sent.'