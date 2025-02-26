#include <GL/gl.h>
#include <GL/glut.h>
#include <SFML/Audio.hpp> // SFML for sound
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <vector>

sf::Sound sound;
sf::SoundBuffer buffer;

sf::Sound sound2;
sf::SoundBuffer buffer2;
// Constants for game mechanics
const int ground_lv = 100;
const int max_Height = 50;
const int max_width = 20;
const float gravity = 3.2;

// Player variables
float pos_y = ground_lv;
float pos_x = 80.0;
float in_vel = 0.4;
int health = 5;
int score = 0;
int duck_offset = 0;
int height = max_Height - duck_offset;
bool isJumping = false;
int counter = 0;
int timer = 3000;
bool isGameOver = false;
bool immunity = false;
bool dscore = false;
int score_count = 10;
int health_count = 1;
float i_timer = 400;
float d_timer = 400;
int ani1 = 0;
int ani2 = 0;
int ani3 = 0;
float clr = 1.0f;
bool clearColor = false;
bool hasBeenHit = false;

// Enum for different object types
enum GameObj { OBSTACLE, COLLECTIBLE, IMMUNITY, SLOWDOWN };

// Object struct for game entities
struct Obj {
  float x;
  float y;
  int height;
  int width;
  GameObj typ;
  bool isActive;
};

// Coordinates for object positions
float coord[10] = {803.0f,  850.0f,  920.0f,  970.0f,  1050.0f,
                   1100.0f, 1150.0f, 1200.0f, 1250.0f, 1300.0f};

// Vector to store objects
std::vector<Obj> obj;

// Function to generate random Y-coordinate for objects
float randomY() {
  int ySet[] = {0, 40, 80};
  return ySet[rand() % 3]; // Randomly pick from {86, 50, 120}
}

// Function to generate random object types
GameObj randomType() {
  int randomChoice =
      rand() % 4 + 1; // Random between {COLLECTIBLE, IMMUNITY, SLOWDOWN}
  return static_cast<GameObj>(randomChoice);
}
void swapEnds() {
  int n = obj.size();
  if (n > 1) { // Ensure there are at least two elements to swap
    std::swap(obj[0], obj[n - 1]); // Swap the first and last objects
  }
}

// Initialize objects at game start
void initObj() {
  // Load background sound
  if (!buffer2.loadFromFile("retrobg.wav")) { // Load your background sound file
    std::cerr << "Failed to load background sound file!" << std::endl;
  }
  sound2.setBuffer(buffer2); // Attach the sound buffer to the sound object
  sound2.setLoop(true);      // Set the sound to loop
  sound2.play();             // Play the background sound
  sound2.setVolume(5);
  // Generate between 5 to 7 objects
  int numObjects = 5 + rand() % 3;

  obj.resize(numObjects); // Resize vector to the required size

  int obstacleCount = 0;

  for (int i = 0; i < numObjects; ++i) {
    obj[i].x = coord[i];
    obj[i].y = randomY() + ground_lv;
    obj[i].height = 50; // Example height, modify as necessary
    obj[i].width = 50;  // Example width, modify as necessary
    obj[i].isActive = true;

    // Ensure there are exactly 2 OBSTACLE objects
    if (obstacleCount < 2 && i == numObjects - 3) {
      obj[i].typ = OBSTACLE;
      obstacleCount++;
    } else {
      obj[i].typ = randomType();
    }
  }
}

// Generate new objects when needed
void generateObjects() {
  if (!obj.empty() && obj.back().x < 0) {
    obj.clear();
    int numObjects = 5 + rand() % 3;
    obj.resize(numObjects);

    int obstacleCount = 0;
    for (int i = 0; i < numObjects; ++i) {
      obj[i].x = coord[i];
      obj[i].y = randomY() + ground_lv;
      obj[i].height = 50;
      obj[i].width = 50;
      obj[i].isActive = true;

      if (obstacleCount < 2 && i >= numObjects - 3) {
        obj[i].typ = OBSTACLE;
        obstacleCount++;
      } else {
        obj[i].typ = randomType();
      }
    }
  }
}

// Shuffle objects for randomness
void shuffleObjects() {
  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(obj.begin(), obj.end(), g);
}

void drawCircle(float width, float height, int segments) {
  float radius = std::min(width, height) /
                 2.0f; // Use the smaller dimension for the radius
  glBegin(GL_TRIANGLE_FAN);
  glVertex2f(0.0f, 0.0f); // Center of the circle
  for (int i = 0; i <= segments; ++i) {
    float angle =
        2.0f * M_PI * i / segments; // Calculate the angle for each segment
    glVertex2f(radius * cos(angle),
               radius * sin(angle)); // Calculate the vertex position
  }
  glEnd();
}

void drawDiamond(float width, float height) {
  float halfWidth = width / 2.0f;   // Half of the width
  float halfHeight = height / 2.0f; // Half of the height
  glBegin(GL_QUADS);
  glVertex2f(0.0f, halfHeight);  // Top vertex (0, halfHeight)
  glVertex2f(halfWidth, 0.0f);   // Right vertex (halfWidth, 0)
  glVertex2f(0.0f, -halfHeight); // Bottom vertex (0, -halfHeight)
  glVertex2f(-halfWidth, 0.0f);  // Left vertex (-halfWidth, 0)
  glEnd();
}

void drawTriangle(float width, float height) {
  float halfWidth = width / 2.0f; // Half of the width
  glBegin(GL_TRIANGLES);
  glVertex2f(0.0f, height);     // Top vertex (0, height)
  glVertex2f(-halfWidth, 0.0f); // Bottom left vertex (-halfWidth, 0)
  glVertex2f(halfWidth, 0.0f);  // Bottom right vertex (halfWidth, 0)
  glEnd();
}
void drawSquare(float width, float height) {
  glBegin(GL_QUADS);
  glVertex2f(-width / 2.0f, -height / 2.0f);
  glVertex2f(width / 2.0f, -height / 2.0f);
  glVertex2f(width / 2.0f, height / 2.0f);
  glVertex2f(-width / 2.0f, height / 2.0f);
  glEnd();
}

void drawRay(float length, float width) {
  // Draw a rectangular ray extending out from the center
  glBegin(GL_QUADS);                 // Rectangular rays
  glVertex2f(0.0f, -width / 2.0f);   // Bottom-left of the ray
  glVertex2f(0.0f, width / 2.0f);    // Top-left of the ray
  glVertex2f(length, width / 2.0f);  // Top-right of the ray
  glVertex2f(length, -width / 2.0f); // Bottom-right of the ray
  glEnd();
}
void drawCircle(float radius, int segments) {
  glBegin(GL_TRIANGLE_FAN);
  glVertex2f(0.0f, 0.0f); // Center of the circle
  for (int i = 0; i <= segments; ++i) {
    float angle = 2.0f * M_PI * i / segments;
    glVertex2f(radius * cos(angle), radius * sin(angle));
  }
  glEnd();
}

void drawSun(float coreRadius, float rayLength, float rayWidth, int rayCount,
             float rotation = 0.0f) {
  glPushMatrix();

  // Rotate the sun rays if needed
  glRotatef(rotation, 0.0f, 0.0f, 1.0f);

  // Draw the rays (spikes)
  float angleStep = 360.0f / rayCount; // Step for each ray
  for (int i = 0; i < rayCount; ++i) {
    glPushMatrix();
    glRotatef(i * angleStep, 0.0f, 0.0f, 1.0f); // Rotate each ray

    // Draw a single ray as a rectangle (vertical, stretching outwards)
    glBegin(GL_QUADS);
    glVertex2f(-rayWidth / 2.0f, coreRadius); // Bottom left
    glVertex2f(rayWidth / 2.0f, coreRadius);  // Bottom right
    glVertex2f(rayWidth / 2.0f,
               coreRadius + rayLength); // Top right (extend out)
    glVertex2f(-rayWidth / 2.0f,
               coreRadius + rayLength); // Top left (extend out)
    glEnd();

    glPopMatrix();
  }

  // Draw the core of the sun (central circle)
  glColor3f(1.0f, 0.8f, 0.0f); // Sun color
  drawCircle(coreRadius, 50);  // 50 segments for smooth circle

  glPopMatrix();
}

void drawRect(float len, float wid) {
  glBegin(GL_QUADS);
  glVertex2d(0.0f, 0.0f);
  glVertex2d(0.0f, len);
  glVertex2d(wid, len);
  glVertex2d(wid, 0.0f);
  glEnd();
}
void drawCircle(float x, float y, float radius, int numSegments) {
  glBegin(GL_TRIANGLE_FAN);
  glVertex2f(x, y); // Center of circle
  for (int i = 0; i <= numSegments; i++) {
    float theta = 2.0f * 3.14159f * float(i) / float(numSegments);
    float dx = radius * cosf(theta);
    float dy = radius * sinf(theta);
    glVertex2f(x + dx, y + dy);
  }
  glEnd();
}

void drawSquareWithSpikes(float width, float height) {
  // Draw the square centered at (0, 0)
  glPushMatrix();
  drawSquare(width, height);
  glPopMatrix();

  // Calculate the half-width and half-height
  float halfWidth = width / 2.0f;
  float halfHeight = height / 2.0f;

  // Spike size (adjust the proportion as needed)
  float spikeHeight = height / 7.0f;

  // Draw top spike
  glPushMatrix();
  glTranslatef(0.0f, halfHeight, 0.0f);    // Move to the middle of the top edge
  glRotatef(0.0f, 0.0f, 0.0f, 1.0f);       // No rotation for the top spike
  drawTriangle(width / 2.0f, spikeHeight); // Triangle spike
  glPopMatrix();

  // Draw bottom spike
  glPushMatrix();
  glTranslatef(0.0f, -halfHeight,
               0.0f); // Move to the middle of the bottom edge
  glRotatef(180.0f, 0.0f, 0.0f,
            1.0f); // Rotate the spike 180 degrees for bottom
  drawTriangle(width / 2.0f, spikeHeight);
  glPopMatrix();

  // Draw left spike
  glPushMatrix();
  glTranslatef(-halfWidth, 0.0f, 0.0f); // Move to the middle of the left edge
  glRotatef(90.0f, 0.0f, 0.0f, 1.0f);   // Rotate the spike 90 degrees
  drawTriangle(height / 2.0f, spikeHeight);
  glPopMatrix();

  // Draw right spike
  glPushMatrix();
  glTranslatef(halfWidth, 0.0f, 0.0f); // Move to the middle of the right edge
  glRotatef(-90.0f, 0.0f, 0.0f, 1.0f); // Rotate the spike -90 degrees
  drawTriangle(height / 2.0f, spikeHeight);
  glPopMatrix();
}

void drawHeart(float width, float height) {
  glBegin(GL_TRIANGLE_FAN);

  // Move to the top of the heart
  glVertex2f(0.0f, height / 4);

  // Draw the heart shape using polar coordinates
  const int num_segments = 100; // Number of segments for smoother shape
  for (int i = 0; i <= num_segments; ++i) {
    float angle = i * (2.0f * M_PI / num_segments);
    float heartX = (16 * pow(sin(angle), 3)); // Heart shape equation (x)
    float heartY = (13 * cos(angle) - 5 * cos(2 * angle) - 2 * cos(3 * angle) -
                    cos(4 * angle)); // Heart shape equation (y)

    // Scale the heart based on the provided width and height
    glVertex2f(heartX * (width / 32), heartY * (height / 32));
  }
  glEnd();
}

void drawSpikedRectangle(float length, float width, int spikes) {
  float spikeWidth = width / spikes; // Width of each spike
  float spikeHeight = width;         // Height of each spike
  float circleRadius =
      spikeWidth / 4.0f; // Circle radius at the tip of each spike

  // Draw the base rectangle
  glBegin(GL_QUADS);
  glVertex2f(0, 0);
  glVertex2f(length, 0);
  glVertex2f(length, width);
  glVertex2f(0, width);
  glEnd();

  // Draw spikes
  for (int i = 0; i < spikes; i++) {
    float x = i * spikeWidth;
    glBegin(GL_TRIANGLES);
    glVertex2f(x, width); // Left point of the spike
    glVertex2f(x + spikeWidth / 2.0f, width + spikeHeight); // Tip of the spike
    glVertex2f(x + spikeWidth, width); // Right point of the spike
    glEnd();

    // Draw circle at the tip of the spike
    drawCircle(x + spikeWidth / 2.0f, width + spikeHeight, circleRadius, 20);
  }
}
void drawPatrickStar(float width, float height) {
    // Body color (light pink)
    //glColor3f(1.0f, 0.75f, 0.8f);
    glColor3f(0.9f, 0.44f, 0.64f);  // Dark pink
    // Draw body (main triangle)
    glBegin(GL_TRIANGLES);
    glVertex2f(0.5f * width, 0.9f * height);  // Top point of the star
    glVertex2f(0.2f * width, 0.1f * height);  // Bottom left
    glVertex2f(0.8f * width, 0.1f * height);  // Bottom right
    glEnd();

    // Left arm (triangle)
    glBegin(GL_TRIANGLES);
    glVertex2f(0.2f * width, 0.6f * height);
    glVertex2f(0.0f * width, 0.4f * height);
    glVertex2f(0.3f * width, 0.4f * height);
    glEnd();

    // Right arm (triangle)
    glBegin(GL_TRIANGLES);
    glVertex2f(0.8f * width, 0.6f * height);
    glVertex2f(1.0f * width, 0.4f * height);
    glVertex2f(0.7f * width, 0.4f * height);
    glEnd();

    // Left leg (triangle)
    glBegin(GL_TRIANGLES);
    glVertex2f(0.4f * width, 0.1f * height);
    glVertex2f(0.35f * width, 0.0f * height);
    glVertex2f(0.45f * width, 0.0f * height);
    glEnd();

    // Right leg (triangle)
    glBegin(GL_TRIANGLES);
    glVertex2f(0.6f * width, 0.1f * height);
    glVertex2f(0.55f * width, 0.0f * height);
    glVertex2f(0.65f * width, 0.0f * height);
    glEnd();

    // Draw eyes (white circles)
    glColor3f(1.0f, 1.0f, 1.0f);
    drawCircle(0.45f * width, 0.75f * height, 0.05f * width, 20);
    drawCircle(0.55f * width, 0.75f * height, 0.05f * width, 20);

    // Draw pupils (black circles)
    glColor3f(0.0f, 0.0f, 0.0f);
    drawCircle(0.45f * width, 0.75f * height, 0.02f * width, 20);
    drawCircle(0.55f * width, 0.75f * height, 0.02f * width, 20);
}
// Draw the player on screen
void drawPlayer() {
  // Draw the body
  glColor3f(1.0f, clr, clr);
  glPushMatrix();

  // Body
  glBegin(GL_QUADS);
  glVertex2d(0.0f, 0.0f);
  glVertex2d(0.0f, height);
  glVertex2d(max_width, height);
  glVertex2d(max_width, 0.0f);
  glEnd();

  // Draw the head
  glPushMatrix();
  glTranslatef(max_width / 2.0f, height, 0.0f); // Position above the body
  float head_radius = max_width / 4.0f;         // Adjust head size as needed
  glBegin(GL_TRIANGLE_FAN);
  glVertex2f(0.0f, 0.0f); // Center of the head
  int num_segments = 20;  // Number of segments for the circle
  for (int i = 0; i <= num_segments; i++) {
    float angle = 2.0f * M_PI * i / num_segments; // Full circle
    float x = head_radius * cos(angle);
    float y = head_radius * sin(angle);
    glVertex2f(x, y); // Add vertex for the circle
  }
  glEnd();
  glPopMatrix();

  // Draw the arms
  float arm_width = max_width / 6.0f; // Adjust arm size as needed
  float arm_height = height / 2.0f;   // Arm height

  // Left arm
  glPushMatrix();
  glTranslatef(-arm_width, height / 2.0f, 0.0f); // Position left arm
  glBegin(GL_QUADS);
  glVertex2d(0.0f, 0.0f);
  glVertex2d(0.0f, arm_height);
  glVertex2d(arm_width, arm_height);
  glVertex2d(arm_width, 0.0f);
  glEnd();
  glPopMatrix();

  // Right arm
  glPushMatrix();
  glTranslatef(max_width, height / 2.0f, 0.0f); // Position right arm
  glBegin(GL_QUADS);
  glVertex2d(0.0f, 0.0f);
  glVertex2d(0.0f, arm_height);
  glVertex2d(arm_width, arm_height);
  glVertex2d(arm_width, 0.0f);
  glEnd();
  glPopMatrix();

  glPopMatrix(); // Pop the player transformation matrix
  //   glColor3f(1.0f, 1.0f, 1.0f);
  //   glPushMatrix();
  //   glTranslatef(pos_x, pos_y, 0.0f);
  //   glBegin(GL_QUADS);
  //   glVertex2d(0.0f, 0.0f);
  //   glVertex2d(0.0f, height);
  //   glVertex2d(max_width, height);
  //   glVertex2d(max_width, 0.0f);
  //   glEnd();
  //   glPopMatrix();
}

float scl = 1;
float theta = 0.0f;      // The vertical position offset
float speed = 0.05f;     // Speed of movement
float maxOffset = 20.0f; //
float clr2 = 0;
// Draw game objects on screen
void drawObj() {
  for (const auto &o : obj) {
    if (o.isActive || o.typ == OBSTACLE) {
      glPushMatrix();
      glTranslatef(o.x, o.y, 0.0f);

      if (o.typ == COLLECTIBLE) {
        glColor3f(1.0f, 0.8f, 0.7f); // peach
        glTranslatef(0, 30, 0);
        glRotated(ani1, 0.0f, 0.0f, 0.4);
        drawDiamond(o.width, o.height);
        glColor3f(0.6f, 1.0f, 0.8f);
        glRotated(-ani1 + 30, 0.0f, 0.0f, 0.2);
        drawTriangle(o.width - 20, o.height - 20);
        glColor3d(1.0f, 1.0f, 0.6f);
        drawCircle(30, 30, 12);
      } else if (o.typ == OBSTACLE) {
        glColor3f(0.8f, 0.0f, 0.0f);
        drawTriangle(o.width, o.height);
        glTranslated(0, 20, 0);
        drawCircle(50, 50, 12);
        glPopMatrix();
      } else if (o.typ == IMMUNITY) {
        glColor3f(0.3f, clr2, 0.2f);
        glTranslatef(0, 30 + theta,
                     0); // Translate using theta for vertical movement
        drawSquareWithSpikes(30.0f, 30.0f); // Draw a square with spikes
      } else if (o.typ == SLOWDOWN) {
        glColor3f(0.4f, 0.2f, 0.5f);
        glTranslatef(0, 30, 0); // Move to the position
        glScalef(scl, scl, 0);
        drawSquareWithSpikes(30.0f, 30.0f);
      }

      //   glBegin(GL_QUADS);
      //   glVertex2d(0.0f, 0.0f);
      //   glVertex2d(0.0f, o.height);
      //   glVertex2d(o.width, o.height);
      //   glVertex2d(o.width, 0.0f);
      //   glEnd();
      glPopMatrix();
    }
  }
}

// Draw the ground on screen
void drawGround() {
  glColor3f(1.0f, 1.0f, 1.0f);
  glPushMatrix();
  glBegin(GL_QUADS);
  glVertex2d(0.0f, 0.0f);
  glVertex2d(0.0f, ground_lv);
  glVertex2d(800.0f, ground_lv);
  glVertex2d(800.0f, 0.0f);
  glEnd();
  glPopMatrix();
}

void drawHUD() {
  // Set text color
  glColor3f(1.0f, 1.0f, 1.0f); // White color for the HUD text

  // Display Health
  std::string healthText = "Health: 💛" + std::to_string(health);
  glRasterPos2f(50, 540); // Position at top-left corner
  for (char c : healthText) {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c); // Render each character
  }

  // Display Score
  std::string scoreText = "Score: " + std::to_string(score);
  glRasterPos2f(50, 520); // Position slightly below health
  for (char c : scoreText) {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
  }

  // Display Timer
  std::string timerText = "Time: " + std::to_string(timer / 60) + "s";
  glRasterPos2f(50, 500); // Position below score
  for (char c : timerText) {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
  }

  if (immunity) {
    std::string i_timerText = "Immunity: " + std::to_string(i_timer / 60) + "s";
    glRasterPos2f(400, 520); // Position below score
    for (char c : i_timerText) {
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
  }
  if (dscore) {
    std::string d_timerText =
        "Double Score: " + std::to_string(d_timer / 60) + "s";
    glRasterPos2f(400, 540); // Position below score
    for (char c : d_timerText) {
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
  }
}

void drawHeart(float x, float y, float width, float height) {
  glBegin(GL_TRIANGLE_FAN);
  glColor3f(1.0f, 0.0f, 0.0f); // Set color to red

  // Move to the top of the heart
  glVertex2f(x, y + (height / 4));

  // Draw the heart shape using polar coordinates
  const int num_segments = 100; // Number of segments for smoother shape
  for (int i = 0; i <= num_segments; ++i) {
    float angle = i * (2.0f * M_PI / num_segments);
    float heartX = (16 * pow(sin(angle), 3)); // Heart shape equation (x)
    float heartY = (13 * cos(angle) - 5 * cos(2 * angle) - 2 * cos(3 * angle) -
                    cos(4 * angle)); // Heart shape equation (y)

    // Scale the heart based on the provided width and height and translate it
    // to (x, y)
    glVertex2f(x + heartX * (width / 32), y + heartY * (height / 32));
  }
  glEnd();
}
void drawGameOver() {
  std::string gameOverText = "Game Over!";
  std::string finalScore = "Final Score: " + std::to_string(score);

  glColor3f(1.0f, 0.0f, 0.0f); // Red color for "Game Over"

  // Draw "Game Over" text
  glRasterPos2f(350, 300); // Position at the center of the screen
  for (char c : gameOverText) {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
  }

  // Draw Final Score
  glRasterPos2f(350, 270); // Slightly below "Game Over" message
  for (char c : finalScore) {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
  }
}
// Check collision between player and object
bool CheckCollision(Obj o) {
  return pos_x + max_width > o.x && pos_x < o.x + o.width &&
         pos_y < o.y + o.height && pos_y + height > o.y;
}

// Handle collision with objects
void handleCollision(Obj &o) {
  if (CheckCollision(o) && o.isActive) {
    switch (o.typ) {
    case OBSTACLE:
      if (!immunity) {
        health -= health_count;
        clearColor = true;
        isJumping = true;
        if (!buffer.loadFromFile("10.wav")) {
          std::cerr << "Failed to load sound file!" << std::endl;
        }
        o.isActive = false;
        sound.setBuffer(buffer); // Attach the sound buffer to the sound object
        sound.play();
      }

      break;
    case COLLECTIBLE:
      if (dscore) {
        score += score_count * 2;
      } else {
        score += score_count;
      }
      if (!buffer.loadFromFile("2.wav")) {
        std::cerr << "Failed to load sound file!" << std::endl;
      }

      sound.setBuffer(buffer); // Attach the sound buffer to the sound object
      sound.play();
      o.isActive = false;
      break;
    case IMMUNITY:
      // Add immunity logic
      immunity = true;
      i_timer += 60;
      o.isActive = false;
      if (!buffer.loadFromFile("4.wav")) {
        std::cerr << "Failed to load sound file!" << std::endl;
      }

      sound.setBuffer(buffer); // Attach the sound buffer to the sound object
      sound.play();
      break;
    case SLOWDOWN:
      // Add slowdown logic
      dscore = true;
      d_timer += 60;
      o.isActive = false;
      if (!buffer.loadFromFile("1.wav")) {
        std::cerr << "Failed to load sound file!" << std::endl;
      }

      sound.setBuffer(buffer); // Attach the sound buffer to the sound object
      sound.play();
      break;
    }
  }
}

// Handle key press events
void KeyPress(unsigned char key, int x, int y) {
  switch (key) {
  case 'w':
    isJumping = true;
    break;
  case 's':
    duck_offset = 25;
    height = max_Height - duck_offset;
    break;
  }
}

void spe(int k, int x,
         int y) // keyboard special key function takes 3 parameters
{
  if (k == GLUT_KEY_UP) // if the up arrow is pressed, then the object will be
                        // translated in the y axis by 10. (moving upwords)
    isJumping = true;
  if (k ==
      GLUT_KEY_DOWN) { // if the down arrow is pressed, then the object will be
    duck_offset = 25;
    height = max_Height - duck_offset;
  }
}
void speUp(int k, int x, int y) // keyboard special key function is called
                                // whenever the special key is released.
{
  if (k == GLUT_KEY_DOWN) { // if the F1 key is released, then the object will
                            // return back to it's original color.
    duck_offset = 0;
    height = max_Height;
  }
}
// Handle key release events
void keyUp(unsigned char key, int x, int y) {
  if (key == 's') {
    duck_offset = 0;
    height = max_Height;
  }
}
float hearts[5] = {50, 80, 110, 140, 170};
// Game display function
void display() {
  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f(0.5f, 0.7f, 1.0f);
  glPushMatrix();
  glTranslatef(400.0f, 300.0f, 0);
  drawSquare(900.0f, 700.0f);
  glPopMatrix();
  glColor3f(1.0f, 1.0f, 0.7f);
  glPushMatrix();
  glTranslated(30, 500, 0);
  drawSun(300.0f, 30.0f, 5.0f, 12,
          ani3); // coreRadius, rayLength, rayWidth, rayCount, rotation
  glPopMatrix();

  drawGround();
  for (int i = 1; i <= health; i++) {
    glPushMatrix();
    drawHeart(hearts[i - 1] + 15, 480, 30, 30);
    glPopMatrix();
  }
  drawObj();
  glPushMatrix();
  glTranslatef(pos_x, pos_y, 0.0);
  drawPlayer();
  glPopMatrix();

  if (isGameOver) {
    drawGameOver(); // Show Game Over screen when the game ends
  } else {
    drawHUD(); // Draw the HUD with health, score, and timer
  }

  glPushMatrix();
  glColor3f(0.0f, 0.0f, 0.0f);
  drawRect(25, 800);
  glPopMatrix();
  glPushMatrix();
  glColor3f(0.0f, 0.0f, 0.0f);
  glTranslatef(0.0f, 580.0f, 0.0f);
  drawRect(20, 800);
  glPopMatrix();
  // glPushMatrix();
  // glColor3f(0.0f, 0.0f, 0.0f);
  // drawRect(800, 20);
  // glPopMatrix();
  // glPushMatrix();
  // glColor3f(0.0f, 0.0f, 0.0f);
  // glTranslatef(780, 0.0f, 0.0f);
  // drawRect(800, 20);
  // glPopMatrix();
  for(int i = 800; i>0; i-=40){
      glPushMatrix();
      glTranslatef(i - 30,10, 0.0f);
      drawPatrickStar(40, 40);
      glPopMatrix();
  }
  for(int i = 800; i>0; i-=40){
      glPushMatrix();
      glTranslatef(i - 30,560, 0.0f);
      drawPatrickStar(40, 40);
      glPopMatrix();
  }
  // for(int i = 600; i>0; i-=40){
  //     glPushMatrix();
  //     glTranslatef(0, i,0.0f);
  //     drawPatrickStar(20, 20);
  //     glPopMatrix();
  // }
  // for(int i = 600; i>0; i-=40){
  //     glPushMatrix();
  //     glTranslatef(780, i,0.0f);
  //     drawPatrickStar(20, 20);
  //     glPopMatrix();
  // }
  
  glutSwapBuffers();
  glFlush();
}

// Update game logic on timer
void updateGame(int value) {
  if (isJumping) {
    pos_y += 10;
    if (pos_y >= ground_lv + max_Height + 80) {
      isJumping = false;
    }
  } else if (pos_y > ground_lv) {
    pos_y -= gravity;
  }

  generateObjects();
  counter++;
  for (auto &o : obj) {
    o.x -= in_vel;
    handleCollision(o);
  }

  if (counter % 50 == 0) {
    in_vel += 0.3;
  }
  if (scl < 1.3) {
    scl += 0.02;
  }
  if (scl >= 1.3) {
    scl = 1;
  }

  if (theta >= maxOffset) {
    speed = -0.05f; // Reverse direction
  } else if (theta <= -maxOffset) {
    speed = 0.05f; // Reverse direction
  }
  theta += speed; // Update position

  timer--;
  if (timer <= 0 || health <= 0) {
    while (sound.getStatus() == sf::Sound::Playing) {
      // You can also handle events here or do other processing
      sf::sleep(sf::milliseconds(10));
    }
    if (!buffer.loadFromFile("12.wav")) {
      std::cerr << "Failed to load sound file!" << std::endl;
    }

    sound.setBuffer(buffer); // Attach the sound buffer to the sound object
    sound.play();
    isGameOver = true; // Trigger Game Over if health or time runs out
    in_vel = 0;
  }

  if (immunity) {
    i_timer--;
  }
  if (dscore) {
    d_timer--;
  }
  if (d_timer <= 0) {
    dscore = false;
  }
  if (i_timer <= 0) {
    immunity = false;
  }
  ani1 += 15;
  if (ani1 >= 360) {
    ani1 = 0;
  }
  ani2 += 5;
  if (ani2 >= 360) {
    ani2 = 0;
  }
  ani3 += 1;
  if (ani3 >= 360) {
    ani3 = 0;
  }
  if (clearColor) {
    pos_x -= 15;
    clr = 0.0;
    clearColor = false;
  }
  if (clr < 1.0) {
    clr += 0.1;
  }
  if (clr >= 1.0) {
    pos_x = 30;
  }
  if (clr2 != 1.0) {
    clr2 += 0.1;
  }
  if (clr2 >= 1.0) {
    clr2 = 0.0;
  }
  glutPostRedisplay();
  glutTimerFunc(16, updateGame, 0);
}

// Main function
int main(int argc, char **argv) {
  srand(static_cast<unsigned>(time(0))); // Initialize random seed
  initObj();
  for (const auto &o : obj) {
    std::cout << "x: " << o.x << ", y: " << o.y << ", type: " << o.typ << '\n';
  }
  // Initialize GLUT
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(800, 600);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Endless Runner Game");

  // Register callbacks
  glutDisplayFunc(display);
  glutKeyboardFunc(KeyPress);
  glutKeyboardUpFunc(keyUp);
  glutSpecialFunc(spe); // call the keyboard special keys function
  glutSpecialUpFunc(speUp);
  glutTimerFunc(16, updateGame, 0);

  // Basic OpenGL setup
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, 800.0, 0.0, 600.0);
  glMatrixMode(GL_MODELVIEW);

  // Main game loop
  glutMainLoop();

  return 0;
}
