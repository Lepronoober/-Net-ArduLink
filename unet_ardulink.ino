// === CONFIGURATION ===
const int pwmPin = 9;   // PWM output
const int rxPin  = 2;   // Digital input connected to other Arduino's PWM
int myID = 0;           // 0 = request DHCP, otherwise static ID

// === VARIABLES ===
int seqNum = 0;
bool connected = false;

// DHCP server state
const int MAX_CLIENTS = 10;
int assignedIDs[MAX_CLIENTS];   // store allocated IDs
int numAssigned = 0;

// === PWM UTILITIES ===
uint8_t readPWMbyte(int pin) {
  unsigned long high = pulseIn(pin, HIGH, 20000);
  unsigned long low  = pulseIn(pin, LOW, 20000);
  if (high == 0 && low == 0) return 0;
  unsigned long period = high + low;
  if (period == 0) return 0;
  float duty = (float)high / (float)period;
  uint8_t val = (uint8_t)round(duty * 255.0);
  Serial.print("[RX] Symbol: ");
  Serial.println(val);
  return val;
}

void sendPWMbyte(uint8_t value) {
  Serial.print("[TX] Symbol: ");
  Serial.println(value);
  analogWrite(pwmPin, value);
  delay(20);  // symbol duration
}

// === SEND MESSAGE ===
void sendMessage(String msg) {
  int len = msg.length();
  int crc = myID + seqNum + len;
  for (int i = 0; i < len; i++) crc = (crc + msg[i]) % 256;

  Serial.println("=== TX Message Start ===");
  Serial.print("  ID   : "); Serial.println(myID);
  Serial.print("  SEQ  : "); Serial.println(seqNum);
  Serial.print("  LEN  : "); Serial.println(len);
  Serial.print("  DATA : "); Serial.println(msg);
  Serial.print("  CRC  : "); Serial.println(crc);

  sendPWMbyte(myID);
  sendPWMbyte(seqNum);
  sendPWMbyte(len);
  for (int i = 0; i < len; i++) sendPWMbyte(msg[i]);
  sendPWMbyte(crc);

  analogWrite(pwmPin, 0);
  delay(50);
  Serial.println("=== TX Message End ===");
}

// === RECEIVE MESSAGE ===
bool receiveMessage(int &sender, int &seq, String &data) {
  int id = readPWMbyte(rxPin);
  if (id == 0) return false;

  int vSEQ = readPWMbyte(rxPin);
  int vLEN = readPWMbyte(rxPin);

  Serial.println("=== RX Message Start ===");
  Serial.print("  ID   : "); Serial.println(id);
  Serial.print("  SEQ  : "); Serial.println(vSEQ);
  Serial.print("  LEN  : "); Serial.println(vLEN);

  String buffer = "";
  int crcCheck = id + vSEQ + vLEN;
  for (int i = 0; i < vLEN; i++) {
    int val = readPWMbyte(rxPin);
    buffer += (char)val;
    crcCheck = (crcCheck + val) % 256;
  }
  int vCRC = readPWMbyte(rxPin);

  Serial.print("  DATA : "); Serial.println(buffer);
  Serial.print("  CRC received : "); Serial.println(vCRC);
  Serial.print("  CRC computed : "); Serial.println(crcCheck);

  if (crcCheck == vCRC) {
    Serial.println("=== RX Message Valid ===");
    sender = id;
    seq = vSEQ;
    data = buffer;
    return true;
  } else {
    Serial.println("=== RX CRC ERROR ===");
  }
  return false;
}

// === SEND ACK ===
void sendACK(int toID, int seq) {
  int crc = myID + seq;
  Serial.println("[TX] Sending ACK");
  Serial.print("  ToID : "); Serial.println(toID);
  Serial.print("  SEQ  : "); Serial.println(seq);
  Serial.print("  CRC  : "); Serial.println(crc);

  sendPWMbyte(myID);
  sendPWMbyte(seq);
  sendPWMbyte(0);
  sendPWMbyte(crc);

  analogWrite(pwmPin, 0);
  delay(50);
  Serial.println("[TX] ACK Sent");
}

// === DHCP-LIKE ID MANAGEMENT ===
int allocateID() {
  for (int id = 1; id < 250; id++) {
    bool used = false;
    for (int i = 0; i < numAssigned; i++) {
      if (assignedIDs[i] == id) {
        used = true;
        break;
      }
    }
    if (!used) {
      if (numAssigned < MAX_CLIENTS) {
        assignedIDs[numAssigned++] = id;
        Serial.print("[DHCP] Allocated ID ");
        Serial.println(id);
        return id;
      }
    }
  }
  Serial.println("[DHCP] No available ID.");
  return 0; // none available
}

void freeID(int id) {
  for (int i = 0; i < numAssigned; i++) {
    if (assignedIDs[i] == id) {
      for (int j = i; j < numAssigned - 1; j++) {
        assignedIDs[j] = assignedIDs[j + 1];
      }
      numAssigned--;
      Serial.print("[DHCP] Released ID ");
      Serial.println(id);
      return;
    }
  }
}

// === DIAL-UP (CLIENT SIDE) ===
void dialUp() {
  Serial.println("[DIAL] Attempting connection...");

  seqNum = (seqNum + 1) % 256;
  sendMessage("DIAL");

  unsigned long start = millis();
  while (millis() - start < 3000) { // wait max 3s
    int sender, seq;
    String msg;
    if (receiveMessage(sender, seq, msg)) {
      if (msg.startsWith("ASSIGN:")) {
        int newID = msg.substring(7).toInt();
        if (newID > 0) {
          myID = newID;
          Serial.print("[DIAL] Assigned ID: ");
          Serial.println(myID);
        } else {
          Serial.println("[DIAL] Assignment failed, no free IDs.");
        }
      } else if (msg == "CONNECT") {
        connected = true;
        Serial.println("[DIAL] Connected.");
        return;
      }
    }
  }
  Serial.println("[DIAL] No response.");
}

// === SERVER ANSWER CALL ===
void answerCall() {
  int sender, seq;
  String msg;
  if (receiveMessage(sender, seq, msg)) {
    if (msg == "DIAL") {
      Serial.println("[CALL] Incoming call.");
      seqNum = (seqNum + 1) % 256;

      // Allocate ID for client
      int newID = allocateID();
      sendMessage("ASSIGN:" + String(newID));
      sendMessage("CONNECT");
      connected = true;
    }
  }
}

// === HANG UP ===
void hangUp() {
  seqNum = (seqNum + 1) % 256;
  sendMessage("BYE");
  freeID(myID);   // release own ID
  connected = false;
  Serial.println("[HANGUP] Session terminated.");
}

// === SETUP ===
void setup() {
  pinMode(pwmPin, OUTPUT);
  pinMode(rxPin, INPUT);
  Serial.begin(9600);

  for (int i = 0; i < MAX_CLIENTS; i++) assignedIDs[i] = 0;

  Serial.print("[BOOT] Arduino started. Current ID=");
  Serial.println(myID);
}

// === LOOP ===
void loop() {
  // 1. Check for incoming calls
  if (!connected) {
    answerCall();
  }

  // 2. Local serial commands
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.equalsIgnoreCase("dial")) {
      if (!connected) dialUp();
    } else if (input.equalsIgnoreCase("bye")) {
      if (connected) hangUp();
    } else if (input.length() > 0 && connected) {
      seqNum = (seqNum + 1) % 256;
      sendMessage(input);
      Serial.print("[LOCAL] Sent: ");
      Serial.println(input);
    }
  }

  // 3. Receive messages during session
  if (connected) {
    int sender, seq;
    String msg;
    if (receiveMessage(sender, seq, msg)) {
      if (msg == "BYE") {
        Serial.println("[REMOTE] Other side hung up.");
        freeID(sender);   // release remote ID
        connected = false;
      } else if (msg != "DIAL" && msg != "CONNECT" && !msg.startsWith("ASSIGN:")) {
        Serial.print("[REMOTE] From ");
        Serial.print(sender);
        Serial.print(": ");
        Serial.println(msg);
        sendACK(sender, seq);
      }
    }
  }
}
