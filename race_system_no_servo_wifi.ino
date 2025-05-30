#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


// Pin definitions
#define LANE1PIN 5   // D1
#define LANE2PIN 14  // D5
#define LANE3PIN 12  // D6
#define LANE4PIN 13  // D7

const char* ssid = "tropicana home_2.4GHz";
const char* password = "jakarta13";  


ESP8266WebServer server(80);


// Race state
volatile bool laneTriggered[4] = {false, false, false, false};
unsigned long finishTimes[4] = {0, 0, 0, 0};
bool raceStarted = false;
bool raceEnded = false;
unsigned long raceStartTime = 0;
unsigned long raceAutoEndTime = 0;
int triggeredCount = 0;

// Result structure
struct Result {
  int lane;
  unsigned long time;
  bool finished;
};

// Function prototypes
void getSortedResults(Result results[], int size);
String getPlaceSuffix(int place);
void resetRace();
void checkLane(int lane);

// Interrupt service routines
void ICACHE_RAM_ATTR lane1ISR() { checkLane(0); }
void ICACHE_RAM_ATTR lane2ISR() { checkLane(1); }
void ICACHE_RAM_ATTR lane3ISR() { checkLane(2); }
void ICACHE_RAM_ATTR lane4ISR() { checkLane(3); }

void checkLane(int lane) {
  if (!laneTriggered[lane]) {
    if (!raceStarted) {
      raceStartTime = millis();
      raceAutoEndTime = raceStartTime + 3000;  // 3 seconds countdown
      raceStarted = true;
      Serial.println("Race started!");
    }
    laneTriggered[lane] = true;
    finishTimes[lane] = millis() - raceStartTime;
  }

  triggeredCount = 0;
  for (int i = 0; i < 4; i++) {
    if (laneTriggered[i]) triggeredCount++;
  }
  if (triggeredCount == 4) {
    raceEnded = true;
    raceStarted = false;
    Serial.println("Race ended (all lanes).");
  }
}

void resetRace() {
  for (int i = 0; i < 4; i++) {
    laneTriggered[i] = false;
    finishTimes[i] = 0;
  }
  raceStarted = false;
  raceEnded = false;
  raceStartTime = 0;
  triggeredCount = 0;
  Serial.println("Race reset.");
}

String getPlaceSuffix(int place) {
  if (place == 1) return "st";
  if (place == 2) return "nd";
  if (place == 3) return "rd";
  return "th";
}

void getSortedResults(Result results[], int size) {
  for (int i = 0; i < size; i++) {
    results[i].lane = i;
    results[i].time = finishTimes[i];
    results[i].finished = laneTriggered[i];
  }

  // Bubble sort
  for (int i = 0; i < size - 1; i++) {
    for (int j = 0; j < size - i - 1; j++) {
      bool swap = false;
      if (!results[j].finished && results[j + 1].finished) {
        swap = true;
      } else if (results[j].finished && results[j + 1].finished && results[j].time > results[j + 1].time) {
        swap = true;
      }

      if (swap) {
        Result temp = results[j];
        results[j] = results[j + 1];
        results[j + 1] = temp;
      }
    }
  }
}

void handleRoot() {
  String html = "<html><head><meta charset='UTF-8'>"
                "<style>"
                "body {font-size:1.2em;font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI Emoji', 'Noto Color Emoji', sans-serif; text-align:center; background:#f0f8ff;}"
"h2 {"
"  font-size: 2.5em;"
"  margin: 20px 0;"
"  display: inline-block;"
"  color: #fff;"
"}"
".wave span {"
"  display: inline-block;"
"  animation: wave 1.5s infinite ease-in-out;"
"  color: #ffa500;"  /* Bright orange */

"}"
".wave span:nth-child(1) { animation-delay: 0s; }"
".wave span:nth-child(2) { animation-delay: 0.1s; }"
".wave span:nth-child(3) { animation-delay: 0.2s; }"
".wave span:nth-child(4) { animation-delay: 0.3s; }"
".wave span:nth-child(5) { animation-delay: 0.4s; }"
".wave span:nth-child(6) { animation-delay: 0.5s; }"
".wave span:nth-child(7) { animation-delay: 0.6s; }"
".wave span:nth-child(8) { animation-delay: 0.7s; }"
".wave span:nth-child(9) { animation-delay: 0.8s; }"
".wave span:nth-child(10) { animation-delay: 0.9s; }"
".wave span:nth-child(11) { animation-delay: 1s; }"
".wave span:nth-child(12) { animation-delay: 1.1s; }"
".wave span:nth-child(13) { animation-delay: 1.2s; }"
".wave span:nth-child(14) { animation-delay: 1.3s; }"
"@keyframes wave {"
"  0%, 100% { transform: translateY(0); }"
"  50% { transform: translateY(-10px); }"
"}"


                ".emoji { font-size: 2.5em; margin-right: 5px; }"
                ".card-container {  display: block; width: 100%; max-width: 300px; margin: 10px auto; padding: 15px; }"
                ".card {width: 220px; margin:30px; padding: 10px; background:#eee; border-radius:10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }"
                ".btn {display:inline-block;margin-top:20px;padding:10px 20px;background:#ff7403;color:white;text-decoration:none;border-radius:5px; }"
                ".top-row,.bottom-row{display:flex;justify-content:center;flex-wrap:wrap;margin:10px;}"
                ".first{background:#ffd700;color:#000;font-size:1.2em; height:110px; border:4px solid #fff;}"
                ".second{background:#c0c0c0;color:#000;}"
                ".third{background:#cd7f32;color:#fff;}"
                ".fourth {background-color: #ccccff;  color: #000066;}"
                ".lane-name{margin-top:10px;font-weight:bold;font-size:1.5em;color:#fff;}"
                ".dnf { background: #000 !important; color: #a00; font-weight: bold; }"
                ".car-animation {  display: flex;  justify-content: space-around;  font-size: 3em;  margin-top: 20px;  animation: moveCars 7s linear infinite;}"
                "@keyframes moveCars {    0% { transform: translateX(-100%) scaleX(-1); }  100% { transform: translateX(100%) scaleX(-1); }                }"
  
                "</style></head><body>";

html += "<h2 class='wave'>🏁 ";
html += "<span>L</span><span>U</span><span>K</span><span>E</span><span>'</span><span>S</span> ";
html += "<span>S</span><span>P</span><span>E</span><span>E</span><span>D</span><span>W</span><span>A</span><span>Y</span> 🏁</h2>";


  // Only include audio tag if race hasn't ended
  if (!raceEnded) {
    html += R"rawliteral(
<audio id="intro" autoplay>
  <source src="https://cdn.jsdelivr.net/gh/tompangalila/letsrace/theme.mp3" type="audio/mpeg">
  Your browser does not support the audio element.
</audio>

<div>
  <button onclick="startAudio()" style="padding:10px 20px; font-size:1.2em;">🔊 Play Sound</button>
</div>

<script>
  const audio = document.getElementById('intro');
  let playCount = 0;
  const maxPlays = 2;

  function startAudio() {
    audio.play();
  }

  audio.addEventListener('ended', () => {
    playCount++;
    if (playCount < maxPlays) {
      audio.currentTime = 0;
      audio.play();
    }
  });

  // Hide the button if autoplay works
  audio.addEventListener('play', () => {
    const btn = document.querySelector('button');
    if (btn) btn.style.display = 'none';
  });
</script>
)rawliteral";
  }

  if (!raceStarted && !raceEnded) {
    html += "<h3>Status: READY...</h3>";

    html += "<div class='card-container'>";
    for (int i = 0; i < 4; i++) {
      int pin = i == 0 ? LANE1PIN : i == 1 ? LANE2PIN : i == 2 ? LANE3PIN : LANE4PIN;
      html += "<div class='card'>🏎️ Lane " + String(i + 1) + " : ";
      html += digitalRead(pin) == HIGH ? "✅ Ready" : "<b>⛔ Blocked</b>";
      html += "</div>";
    }
    html += "</div>";
    html += "<a class='btn' href='/'>🔄 Show Result</a>";
    html += R"rawliteral(
    <div class="car-track">
      <div class="car-animation">🏎️    &nbsp;&nbsp; &nbsp; &nbsp; 🚗  &nbsp; 🚕💨   🚓    &nbsp; &nbsp; &nbsp;   &nbsp;   &nbsp;         🛻  &nbsp;🚙 &nbsp;&nbsp; 🛵 &nbsp;&nbsp; 🚌</div>
      <hr style="border: none; height: 20px; background-color: orange; margin: 20px 0;">
   

    </div>
    )rawliteral";

  } else if (raceStarted && !raceEnded) {
    html += "<h3>Status: Race in progress...</h3>";
    html += "<a class='btn' href='/'>🔄 Show Result</a>";
  } else if (raceEnded) {
    html += R"rawliteral(
<audio id="finishSound" autoplay>
  <source src="https://cdn.jsdelivr.net/gh/tompangalila/letsrace/finish.mp3" type="audio/mpeg">
  Your browser does not support the audio element.
</audio>
<script>
  const finishAudio = document.getElementById('finishSound');
  finishAudio.play().catch(() => {
    // Autoplay might fail; user can interact to play if needed
    const btn = document.createElement('button');
    btn.textContent = "▶️ Play Finish Sound";
    btn.className = "btn";
    btn.onclick = () => finishAudio.play();
    document.body.insertBefore(btn, document.body.firstChild);
  });
</script>
)rawliteral";

    Result results[4];
    getSortedResults(results, 4);

    html += "<h3>Race Ended</h3>";
    html += "<div class='top-row'>";
    if (results[0].finished) {
      html += "<div class='card first'>";
      html += "🥇 1st Place";
      html += "<div class='lane-name'>🏎️ Lane " + String(results[0].lane + 1) + "</div>";
      html += "</div>";
    }
    html += "</div>";

    html += "<div class='bottom-row'>";
    for (int i = 1; i < 4; i++) {
      int place = i + 1;
      int lane = results[i].lane;
      String placeClass;
      if (!results[i].finished) {
        placeClass = "dnf";
      } else if (place == 2) {
        placeClass = "second";
      } else if (place == 3) {
        placeClass = "third";
      } else {
        placeClass = "fourth";
      }

      html += "<div class='card " + placeClass + "'>";
      if (results[i].finished) {
        String emoji;
        if (place == 2) {
          emoji = "🥈";
        } else if (place == 3) {
          emoji = "🥉";
        } else if (place == 4) {
          emoji = "🏁";  
        }
        html += "<span class='emoji'>" + emoji + "</span> " + String(place) + getPlaceSuffix(place) + " Place<br>";
        html += "+ " + String(results[i].time / 1000.0, 3) + " s";
      } else {
        html += "DNF";
      }
      html += "<div class='lane-name'>" + String(results[i].finished ? "🏎️ " : "") + "Lane " + String(lane + 1) + "</div>";
      html += "</div>";
    }
    html += "</div>";

    html += "<a class='btn' href='/reset'>🔁 Reset Race</a>";
  }

  html += "</body></html>";
  server.send(200, "text/html", html);
}


void setup() {
  Serial.begin(115200);
  WiFi.hostname("letsrace");
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("Local IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(LANE1PIN, INPUT_PULLUP);
  pinMode(LANE2PIN, INPUT_PULLUP);
  pinMode(LANE3PIN, INPUT_PULLUP);
  pinMode(LANE4PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(LANE1PIN), lane1ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(LANE2PIN), lane2ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(LANE3PIN), lane3ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(LANE4PIN), lane4ISR, FALLING);

  server.on("/", handleRoot);

  server.on("/reset", []() {
    resetRace();
    server.sendHeader("Location", "/");
    server.send(303);  // Redirect to root
  });

  server.begin();
}

void loop() {
  server.handleClient();

  if (raceStarted && !raceEnded && millis() > raceAutoEndTime) {
    raceEnded = true;
    raceStarted = false;
    Serial.println("Race ended (3 second timeout).");
  }
}
