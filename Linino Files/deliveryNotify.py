# Import smtplib for the actual sending function
import os
import smtplib

# Here are the email package modules we'll need
from email.mime.image import MIMEImage
from email.mime.multipart import MIMEMultipart


# Create the container (outer) email message.
msg = MIMEMultipart()
msg['Subject'] = 'Package Delivered.'
from_addr = 'your-email-address@gmail.com'
PASSWORD = 'yourpassword'
to_addr = 'destination-email-address@gmail.com'
msg['From'] = from_addr
msg['To'] = to_addr
msg.preamble = 'Your package has arrived.'

fileDir = "/mnt/sda1/webcam"   
os.chdir(fileDir)
filelist = os.listdir(os.getcwd())
filelist = filter(lambda x: not os.path.isdir(x), filelist)
attachment = max(filelist, key=lambda x: os.stat(x).st_mtime)

fp = open(attachment, 'rb')
img = MIMEImage(fp.read(), name = "image.jpg")
fp.close()
msg.attach(img)

# Send the email via our own SMTP server.
server = smtplib.SMTP('smtp.gmail.com:587')
server.ehlo_or_helo_if_needed()
server.starttls()
server.ehlo_or_helo_if_needed()
server.login(from_addr,PASSWORD)
server.sendmail(from_addr, to_addr, msg.as_string())
server.quit()

print 'Email sent.'
