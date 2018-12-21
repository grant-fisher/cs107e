# shannonstanford-grant-fisher-JumboJoll-project

This repo contains all of the code written for our final project in CS107E.  

By combining our knowledge of hardware, software, and mechanics, we were able to create a functioning CNC drawing machine that is seamlessly integrated with our code written during CS107E.  

Our project involved assembling and programing a CNC drawing machine from used optical disc drives and controlling it using a Raspberry Pi. Our CNC (computer numerical control) drawing machine uses stepper motors to control the relative position of a pen moving along the x and y axes of a sheet of paper. We were able to get both axes precisely responsive to programs sent to it from the Raspberry Pi and created a library to draw images.

Our final CNC drawing machine has two main features: a graphics library, and a 'draw' mode.

The graphics library is fully integrated with our shell, presenting the user with various commands that gives them the ability to draw any sized N-sided polygon at a location on the paper of their choice.

The 'draw' mode wipes the monitor of all text and prompts the user to draw a picture using the mouse.  Once the user is done drawing, they simply press the right button on the mouse, and whatever they drew on the screen is then drawn onto the paper by our machine. 

The specific libraries we created are located within: axis.c, graphics.c, pen.c, draw.c, and mouse.c. We also implemented our new commands within shell.c as well as created a new version of the original ringbuffer library call ringbufferProject.c which allows us to create a larger ringbuffer. 

Team members and their responsibilities: Ryan Ressmeyer: Hardware - building and wiring the physical machine and writing code to drive stepper motors to move the axes. Grant Fisher: Software - writing high level code to convert drawings to commands that can be interpreted and converted into specific commands for the machine. Shannon Yan: Software - writing mid level code to control absolute position, the mouse, and whether the pen is drawing or not.

To see our project in action, watch this video: https://drive.google.com/file/d/1Qn4i_hO1dxzMcv5MeoyS0x5hsY4syWJa/view?usp=sharing

References: We used this tutorial for help with assembling the hardware: http://www.instructables.com/id/DIY-Arduino-Drawing-Machine/
This tutorial was used for help with stepper motor control
http://www.instructables.com/id/Arduino-How-to-Control-a-Stepper-Motor-With-L293D-/
