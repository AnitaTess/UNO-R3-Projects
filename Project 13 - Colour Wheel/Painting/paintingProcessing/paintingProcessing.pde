import processing.serial.*;

Serial arduino;

String portName = "COM3"; // Change to your Arduino Uno port

int selectedArea = 0;
int totalAreas = 8;

String[] areaNames = {
  "Sky",
  "Sun",
  "Cloud",
  "Flower petals",
  "Flower centre",
  "Stem",
  "Leaf",
  "Ground"
};

color[] areaColours = new color[totalAreas];
boolean[] areaFilled = new boolean[totalAreas];

boolean complete = false;
String statusMessage = "Use NEXT/PREV to choose an area. Press SCAN to colour it.";

void setup() {
  size(1000, 720);
  smooth(8);

  println(Serial.list());

  arduino = new Serial(this, portName, 9600);
  arduino.bufferUntil('\n');

  resetArtwork();

  textAlign(CENTER, CENTER);
}

void draw() {
  background(245);

  drawHeader();
  drawArtwork();
  drawAreaPanel();
  drawStatus();

  if (complete) {
    drawCompleteOverlay();
  }
}

void serialEvent(Serial arduino) {
  String message = trim(arduino.readStringUntil('\n'));

  if (message == null || message.length() == 0) {
    return;
  }

  println(message);

  if (message.equals("PREV")) {
    if (!complete) {
      selectedArea--;
      if (selectedArea < 0) selectedArea = totalAreas - 1;
      statusMessage = "Selected: " + areaNames[selectedArea];
    }
  } 
  else if (message.equals("NEXT")) {
    if (!complete) {
      selectedArea++;
      if (selectedArea >= totalAreas) selectedArea = 0;
      statusMessage = "Selected: " + areaNames[selectedArea];
    }
  } 
  else if (message.equals("SCANNING")) {
    if (!complete) {
      statusMessage = "Scanning colour card for: " + areaNames[selectedArea];
    } else {
      resetArtwork();
    }
  } 
  else if (message.startsWith("COLOR:")) {
    String detectedColour = message.substring(6);

    if (!complete) {
      applyDetectedColour(detectedColour);
    }
  }
}

void resetArtwork() {
  for (int i = 0; i < totalAreas; i++) {
    areaColours[i] = color(255);
    areaFilled[i] = false;
  }

  selectedArea = 0;
  complete = false;
  statusMessage = "Use NEXT/PREV to choose an area. Press SCAN to colour it.";
}

void applyDetectedColour(String colourName) {
  color chosenColour = getColourFromName(colourName);

  areaColours[selectedArea] = chosenColour;
  areaFilled[selectedArea] = true;

  statusMessage = areaNames[selectedArea] + " coloured " + colourName + ".";

  if (allAreasFilled()) {
    complete = true;
    statusMessage = "Artwork complete! Press SCAN to restart.";
  }
}

boolean allAreasFilled() {
  for (int i = 0; i < totalAreas; i++) {
    if (!areaFilled[i]) {
      return false;
    }
  }

  return true;
}

color getColourFromName(String colourName) {
  if (colourName.equals("White")) {
    return color(255);
  } 
  else if (colourName.equals("Yellow")) {
    return color(255, 220, 0);
  } 
  else if (colourName.equals("Orange")) {
    return color(255, 120, 0);
  } 
  else if (colourName.equals("Red")) {
    return color(230, 35, 35);
  } 
  else if (colourName.equals("Purple")) {
    return color(145, 70, 200);
  } 
  else if (colourName.equals("Green")) {
    return color(50, 180, 80);
  } 
  else if (colourName.equals("Blue")) {
    return color(60, 130, 230);
  } 
  else if (colourName.equals("Black")) {
    return color(20);
  }

  return color(255);
}

void drawHeader() {
  fill(30);
  textSize(34);
  text("Arduino Colouring Book", width / 2, 38);

  fill(90);
  textSize(16);
  text("NEXT/PREV selects an area. SCAN reads a colour card and fills the selected area.", width / 2, 70);
}

void drawArtwork() {
  pushMatrix();
  translate(80, 120);

  // Area 0: Sky
  drawAreaRect(0, 0, 0, 620, 380, 30);

  // Area 7: Ground
  drawAreaRect(7, 0, 335, 620, 170, 30);

  // Area 1: Sun
  drawAreaCircle(1, 500, 80, 115);

  // Area 2: Cloud
  drawCloud(2, 145, 90);

  // Area 3: Flower petals
  drawPetals(3, 310, 255);

  // Area 4: Flower centre
  drawAreaCircle(4, 310, 255, 70);

  // Area 5: Stem
  drawStem(5, 310, 290);

  // Area 6: Leaf
  drawLeaf(6, 350, 360);

  popMatrix();
}

void drawAreaRect(int index, float x, float y, float w, float h, float radius) {
  fill(areaColours[index]);
  strokeForArea(index);
  rect(x, y, w, h, radius);
}

void drawAreaCircle(int index, float x, float y, float size) {
  fill(areaColours[index]);
  strokeForArea(index);
  ellipse(x, y, size, size);
}

void drawCloud(int index, float x, float y) {
  fill(areaColours[index]);
  strokeForArea(index);

  ellipse(x, y, 95, 65);
  ellipse(x + 55, y - 15, 95, 75);
  ellipse(x + 110, y, 105, 65);
  rect(x - 5, y, 125, 42, 20);
}

void drawPetals(int index, float cx, float cy) {
  fill(areaColours[index]);
  strokeForArea(index);

  for (int i = 0; i < 8; i++) {
    pushMatrix();
    translate(cx, cy);
    rotate(TWO_PI / 8 * i);
    ellipse(0, -75, 55, 105);
    popMatrix();
  }
}

void drawStem(int index, float x, float y) {
  fill(areaColours[index]);
  strokeForArea(index);
  rect(x - 13, y, 26, 145, 15);
}

void drawLeaf(int index, float x, float y) {
  fill(areaColours[index]);
  strokeForArea(index);

  pushMatrix();
  translate(x, y);
  rotate(radians(-25));
  ellipse(0, 0, 95, 45);
  popMatrix();
}

void strokeForArea(int index) {
  if (index == selectedArea && !complete) {
    stroke(255, 80, 80);
    strokeWeight(6);
  } else {
    stroke(40);
    strokeWeight(2);
  }
}

void drawAreaPanel() {
  int panelX = 745;
  int panelY = 125;
  int panelW = 210;
  int rowH = 50;

  fill(255);
  stroke(210);
  strokeWeight(2);
  rect(panelX, panelY, panelW, 445, 24);

  fill(30);
  textSize(22);
  text("Areas", panelX + panelW / 2, panelY + 35);

  textSize(15);

  for (int i = 0; i < totalAreas; i++) {
    int y = panelY + 70 + i * rowH;

    if (i == selectedArea && !complete) {
      fill(255, 235, 235);
      stroke(255, 80, 80);
      strokeWeight(2);
      rect(panelX + 15, y - 18, panelW - 30, 36, 12);
    }

    noStroke();

    fill(areaColours[i]);
    rect(panelX + 28, y - 12, 24, 24, 6);

    stroke(80);
    strokeWeight(1);
    noFill();
    rect(panelX + 28, y - 12, 24, 24, 6);

    fill(40);
    textAlign(LEFT, CENTER);
    text(areaNames[i], panelX + 62, y);

    if (areaFilled[i]) {
      fill(60, 160, 90);
      text("✓", panelX + panelW - 40, y);
    }
  }

  textAlign(CENTER, CENTER);
}

void drawStatus() {
  fill(30);
  textSize(18);
  text(statusMessage, width / 2, height - 65);

  fill(90);
  textSize(15);
  text("Selected area: " + areaNames[selectedArea], width / 2, height - 35);
}

void drawCompleteOverlay() {
  fill(0, 0, 0, 120);
  rect(0, 0, width, height);

  fill(255);
  stroke(40);
  strokeWeight(3);
  rect(width / 2 - 230, height / 2 - 90, 460, 180, 28);

  fill(30);
  textSize(34);
  text("Artwork Complete!", width / 2, height / 2 - 35);

  textSize(18);
  text("Press the SCAN button to restart.", width / 2, height / 2 + 25);
}
