# File:   README
# Author: T. Hadler, S. Przychodzko
# Date:   17 Nov 2018
# Descr:  readme text file for maze game

Upon making the game the user will be greeted with a screen of 
scrolling text saying 'MAZE!'. To initialise the game one of the two 
players must push the navswitch button without blocking the ir 
communication between the two funkit devices, this will subsequently 
start the game for both players. 

Each player navigates the maze by shifting the navswitch north, south, 
east, and west to go up, down, left and right respectively. The walls 
of the maze are displayed as constantly on lights and are impassable.
The slow flashing light at the centre of the LED grid represents the 
player and the map moves around this point as the player navigates the 
maze. 

The aim of the game is to collect objects within the maze while 
avoiding the enemy that chases you. Objects are visually represented by 
fast flashing stationary lights, whereas the enemy is shown as a fast 
flashing moving light. 

To collect an object the player must position themselves on top of the 
object and push the navswitch button, this will add one to the players 
score, and another object will be randomly generated within the maze. 
If the enemy is able to position itself on top of the player then one 
point is added to the other players score, and the original player is 
moved to a random location within the maze. 

Each time a score change occurs the new score is displayed on the 
screens of both players for 1 second (must not be blocking the ir 
communcation when this score change occurs). For both players it shows 
the score of the user of the device on the left and the score of the 
opponent on the right of the screen. 

The end game occurs when one player gets 4 points, at which point the 
game ends and the game over message is displayed as scrolling text. 
