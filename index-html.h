/** 
 *
 *
 *  \file      index-html.h
 *  \author    Bernhard Stöffler
 *  \date      2021-02-24
 */
#ifndef __INDEX-HTML_H__
#define __INDEX-HTML_H__

const char g_hHead[] = 
"<!DOCTYPE HTML>\
<html>\
  <head>\
    <title>Party Hat Control Center</title>\
    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-15\" />\
    <meta name=\"viewport\" content=\"width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no\"/>\
    <style> input,select { font-size:95%; } body { font-size:140%; } </style>\
  </head>\
  <body bgcolor=\"#ffffff\">\
  <center>\
";

const char g_hTail[] = 
    "<hr>\
	<p><form action=\"/cmd\">\
      <input type=\"submit\" value=\"Switch Mode\">\
      <select name=\"switchMode\">\
	<option value=\"rainbow\">Rainbow</option>\
	<option value=\"circle\">Circle</option>\
	<option value=\"breathe\">Breathe</option>\
	<option value=\"fire\">Fire</option>\
	<option value=\"wave\">Wave</option>\
	<option value=\"noise\">Lava Lamp</option>\
	<option value=\"text\">Running Text</option>\
      </select>\
    </form></p>\
    <p><form action=\"/cmd\">\
      <input type=\"submit\" value=\"Total Brightness\">\
      <input type=\"range\" name=\"overallBrightness\" min=\"1\" max=\"64\" value=\"8\">\
    </form></p>\
  </center>\
  </body>\
</html>";


// color pickers are not indented, since character position is important
// and whitespace should be minimized (the space after "Color: " is required!)
const char g_hColorPickerSingle[] =
"<p><form action=\"/cmd\"> Color: \
<input type=\"color\" name=\"color_1\" value=\"#00ff00\">\
<input type=\"submit\">\
</form></p>";

const char g_hColorPickerMulti[] =
"<p><form action=\"/cmd\"> Colors:\
<input type=\"color\" name=\"color_1\" value=\"#00ff00\">\
<input type=\"color\" name=\"color_2\" value=\"#800080\">\
<input type=\"color\" name=\"color_3\" value=\"#000000\">\
<input type=\"color\" name=\"color_4\" value=\"#000000\">\
<input type=\"color\" name=\"color_5\" value=\"#000000\">\
<input type=\"submit\">\
</form></p>";

const int g_aColorPickerOffset[] =
{74, 125, 176, 227, 278};

const char g_hRainbow[] = 
"<h3>Mode: Rainbow</h3><br>\
<p><form action=\"/cmd\"> Speed: \
<input type=\"range\" name=\"rainbow_speed\" min=\"0\", max=\"100\", value=\"16\">\
</p><p>Slant: \
<input type=\"range\" name=\"rainbow_slant\" min=\"-36\", max=\"36\">\
</p><p><input type=\"submit\"></form></p>\
<hr>\
<p><form action=\"/cmd\">\
<input type=\"submit\" name=\"rainbow_invertDirection\" value=\"Change Direction\">\
</form></p>";

const char g_hCircle[] = 
"<h3>Mode: Circle</h3><br>\
<p><form action=\"/cmd\"> Number:\
	<input type=\"radio\" name=\"circle_number\" value=\"1\">\
	<input type=\"radio\" name=\"circle_number\" value=\"2\">\
	<input type=\"radio\" name=\"circle_number\" value=\"3\">\
	<input type=\"radio\" name=\"circle_number\" value=\"4\">\
	<input type=\"radio\" name=\"circle_number\" value=\"5\">\
</p><p> Color:\
	<input type=\"color\" name=\"circle_color\">\
</p><p> Speed:\
	<input type=\"range\" name=\"circle_speed\" min=\"-64\", max=\"64\">\
 </p><p> Width:\
    <input type=\"range\" name=\"circle_width\" min=\"1\", max=\"20\">	\
</p><p><input type=\"submit\"></form></p>";

const char g_hNoiseHead[] = 
"<h3>Mode: Lava Lamp</h3><br>\
<p><form action=\"/cmd\">\
  <input type=\"submit\" value=\"Switch Type\">\
  <select name=\"noise_switchType\">\
    <option value=\"hue\">Heatmap</option>\
    <option value=\"single\">Single-Color</option>\
    <option value=\"multi\">Multi-Color</option>\
  </select>\
</form></p><hr>";

const char g_hNoiseTail[] =
"<p><form action=\"/cmd\"> Speed:\
  <input type=\"range\" name=\"noise_speed\" min=\"0\" max=\"16\" value=\"4\">\
</p><p> Scale:\
  <input type=\"range\" name=\"noise_scale\" min=\"1\" max=\"16\" value=\"8\">\
</p><p><input type=\"submit\">\
</form></p>";

const char g_hNoiseSingle[] = // todo: merge the two forms into one
"<p><form action=\"/cmd\">Threshold:\
  <input type=\"range\" name=\"noise_threshold\" min=\"0\" max=\"255\" value=\"85\">\
  <input type=\"submit\">\
</form></p>";

const char g_hText[] =
"<h3>Mode: Running Text</h3><br>\
<p><form action=\"/cmd\">Text:\
  <input type=\"text\" name=\"text_text\">\
</form></p>\
<hr>\
<p><form action=\"/cmd\"> Speed:\
  <input type=\"range\" name=\"text_speed\" min=\"0\" max=\"32\" value=\"16\">\
</p><p>Color:\
  <input type=\"color\" name=\"text_color\">\
</p><p>Wrap Offset:\
  <input type=\"range\" name=\"text_wrapOffset\" min=\"1\" max=\"32\" value=\"10\">\
</p><p><input type=\"submit\">\
</p></form>";


#endif /* __INDEX-HTML_H__ */
