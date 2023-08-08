#include "webpage_setup.h"

const char setup_html[] PROGMEM = R"rawliteral(
    <!DOCTYPE html>
<html>
<head>
  <title>ESP32 Configuration</title>
  <style>
    body {
      display: flex;
      flex-direction: row;
      align-items: top;
      justify-content: flex-start;
      text-align: center;
    }

    .section {
      margin: 10px;
      padding: 10px;
      border: 1px solid #ccc;
      border-radius: 5px;
    }

    .flex-container {
      display: flex;
      align-items: center;
      justify-content: flex-start;
      margin-bottom: 5px;
    }

    .topic-input {
      margin-left: 5px;
    }

    button {
      margin-top: 10px;
    }
  </style>
  <script>
    function showForms() {
      var connectivityType = document.getElementById("connectivityType").value;
      var wifiForms = document.getElementById("wifiForms");
      var gsmForms = document.getElementById("gsmForms");

      if (connectivityType === "wifi") {
        wifiForms.style.display = "block";
        gsmForms.style.display = "none";
      } else if (connectivityType === "gsm") {
        wifiForms.style.display = "none";
        gsmForms.style.display = "block";
      } else if (connectivityType === "both") {
        gsmForms.style.display = "block";
        wifiForms.style.display = "block";
      }
    }

    function addTopic() {
      var mqttTopics = document.getElementById("mqttTopics");
      var topicIndex = mqttTopics.childElementCount + 1;

      var flexContainer = document.createElement("div");
      flexContainer.classList.add("flex-container");

      var label = document.createElement("label");
      label.innerText = "Topic " + topicIndex + ":";
      flexContainer.appendChild(label);

      var input = document.createElement("input");
      input.type = "text";
      input.name = "mqtt_topic[]";
      input.classList.add("topic-input");
      flexContainer.appendChild(input);

      mqttTopics.appendChild(flexContainer);
    }
  </script>
</head>
<body>
  <div class="section">
    <h3>Connectivity</h3>
    <select id="connectivityType" onchange="showForms()">
      <option value="wifi">WiFi</option>
      <option value="gsm">GSM</option>
      <option value="both">WiFi & GSM</option>
    </select>

    <div id="wifiForms" style="display: block;">
      <h3>WiFi Credentials</h3>
      <input type="text" name="ssid" placeholder="SSID"><br>
      <input type="password" name="password" placeholder="Password"><br><br>
    </div>

    <div id="gsmForms" style="display: none;">
      <h3>GSM Credentials</h3>
      <select id="modemType">
        <option value="saraR422">Ublox Sara R422</option>
        <option value="Other">Other</option>
      </select>
      <div> 
        <input type="text" name="apn" placeholder="APN"><br>
        <input type="password" name="sim_pin" placeholder="SIM PIN"><br><br>
      </div>
    </div>
  </div>

  <div class="section">
    <h3>MQTT Connection</h3>
    <input type="text" name="client_id" placeholder="Client ID"><br>
    <input type="password" name="password" placeholder="Password"><br>
    <input type="text" name="server_url" placeholder="Server URL"><br><br>
  </div>

  <div class="section">
    <h3>MQTT Topics</h3>
    <div id="mqttTopics">
      <div class="flex-container">
        <label>Topic 1:</label>
        <input type="text" name="mqtt_topic[]" class="topic-input"><br>
      </div>
    </div>
    <button type="button" onclick="addTopic()">+</button><br><br>
  </div>

  <div>
    <button type="button" onclick="submitForms()">Submit</button>
    <button type="button" onclick="submitForms()">Back</button>
  </div>

  <script>
    function submitForms() {
  // Get form values
  var connectivityType = document.getElementById("connectivityType").value;
  var modemType = document.getElementById("modemType").value;
  var ssid = document.querySelector('input[name="ssid"]').value;
  var password = document.querySelector('input[name="password"]').value;
  var apn = document.querySelector('input[name="apn"]').value;
  var simPin = document.querySelector('input[name="sim_pin"]').value;
  var clientId = document.querySelector('input[name="client_id"]').value;
  var mqttPassword = document.querySelector('input[name="password"]').value;
  var serverUrl = document.querySelector('input[name="server_url"]').value;

  var mqttTopics = [];
  var topicInputs = document.querySelectorAll('input[name="mqtt_topic[]"]');
  topicInputs.forEach(function (input) {
    mqttTopics.push(input.value);
  });

  // Prepare form data
  var formData = new FormData();
  formData.append("connectivityType", connectivityType);
  formData.append("modemType", modemType);
  formData.append("ssid", ssid);
  formData.append("password", password);
  formData.append("apn", apn);
  formData.append("simPin", simPin);
  formData.append("clientId", clientId);
  formData.append("mqttPassword", mqttPassword);
  formData.append("serverUrl", serverUrl);
  mqttTopics.forEach(function (topic, index) {
    formData.append("mqttTopics[]", topic);
  });

  // Send form data to ESP32
  var xhr = new XMLHttpRequest();
  xhr.open("POST", "/save", true);
  xhr.onload = function () {
    if (xhr.status === 200) {
      console.log("Form submitted successfully!");
    } else {
      console.error("Error submitting the form.");
    }
  };
  xhr.send(formData);
}
  </script>
</body>
</html>

)rawliteral";