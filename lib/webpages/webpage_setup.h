#pragma once

#ifndef WEBPAGE_SETUP_H
#define WEBPAGE_SETUP_H

#include <pgmspace.h>

const char setup_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>SDR Control Unit Setup</title>
    <style>
        body {
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            background: hsl(var(--background, 20 14.3% 4.1%)); /* Dark background by default */
            color: hsl(var(--foreground, 0 0% 95%)); /* Light foreground by default */
        }
        .light body {
            background: hsl(var(--background, 0 0% 100%));
            color: hsl(var(--foreground, 240 10% 3.9%));
        }
        .form-box {
            width: 300px;
            padding: 20px;
            border: 1px solid hsl(var(--card, 0 0% 100%));
            border-radius: var(--radius, 0.5rem);
            background: hsl(var(--card, 0 0% 100%));
        }
        .dark .form-box {
            border: 1px solid hsl(var(--card, 24 9.8% 10%));
            background: hsl(var(--card, 24 9.8% 10%));
        }
        .form-group {
            margin-bottom: 10px;
        }
        .form-label {
            font-weight: bold;
        }
        .form-input {
            width: 100%;
            padding: 5px;
            border: 1px solid hsl(var(--border, 240 5.9% 90%));
            background: hsl(var(--input, 240 5.9% 90%));
            color: hsl(var(--foreground, 240 10% 3.9%));
        }
        .dark .form-input {
            background: hsl(var(--input, 240 3.7% 15.9%));
            color: hsl(var(--foreground, 0 0% 95%));
        }
        .form-submit {
            margin-top: 10px;
        }
        button {
            background: hsl(var(--primary, 142.1 76.2% 36.3%));
            color: hsl(var(--primary-foreground, 355.7 100% 97.3%));
            border: none;
            padding: 10px 20px;
            border-radius: var(--radius, 0.5rem);
            cursor: pointer;
        }
        .dark button {
            background: hsl(120 100% 30%); /* Green button color for dark mode */
            color: hsl(var(--primary-foreground, 355.7 100% 97.3%));
        }
        button:hover {
            background: hsl(var(--secondary, 240 4.8% 95.9%));
        }
        .dark button:hover {
            background: hsl(var(--primary, 142.1 76.2% 36.3%));
        }
    </style>
</head>
<body class="dark">
    <div class="form-box">
        <form id="setupForm" action="/save" method="get">
            <div class="form-group">
                <label class="form-label" for="ssid">WiFi SSID:</label>
                <input class="form-input" type="text" id="ssid" name="ssid" required>
            </div>
            <div class="form-group">
                <label class="form-label" for="password">Password:</label>
                <input class="form-input" type="password" id="password" name="password" required>
            </div>
            <div class="form-group">
                <label class="form-label" for="setupToken">Setup Token:</label>
                <input class="form-input" type="text" id="setupToken" name="setupToken" required>
            </div>
            <div class="form-submit">
                <button type="submit">Submit</button>
            </div>
        </form>
    </div>
</body>
</html>


)rawliteral";

#endif
