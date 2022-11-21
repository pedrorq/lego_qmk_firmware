#!/usr/bin/env python3

import logging


# ================================= DISCLAIMER =================================
# Should be compatible with +3.6 (f-strings), but only tested on 3.11 and 3.10.8
# ==============================================================================


# ======= CONFIG =======
USAGE         = 0x0058
USAGE_PAGE    = 0xFF51
SCREEN_WIDTH  = 480
SCREEN_HEIGHT = 320
LOG_LEVEL     = logging.CRITICAL
# ======================


logging.basicConfig(
    format="%(asctime)s - %(name)s - %(levelname)s - %(message)s",
    level=LOG_LEVEL,
    # filename="qp_xap.log",
    # filemode="w",
)


# ============================================ HELPERS ============================================
def check_dependencies():
    """This function will check for the needed dependencies.
    If something is not installed, user will be prompted get it installed.
    """

    import importlib
    import subprocess
    global sys
    import sys

    for dep in ["hid", "inquirer"]:
        try:
            importlib.import_module(dep)
            logging.info(f"{dep} was already installed ✔")

        except Exception as e:
            logging.error(f"Error while trying to import {dep}: [{e.__class__.__name__}]{e}")
            ans = input(f"Module {dep} not installed, do you want to install it? [Y/n]: ")
            if ans.lower() in ["", "y", "yes"]:
                print("-------------")
                logging.info(f"Intalling {dep} through pip")
                subprocess.run([sys.executable, "-m", "pip", "install", dep])
                print("-------------")

def to_u8(value):
    return int(value)&0xFF

def parse_color(color):
    color = (
        color.replace("[", "")
             .replace("]", "")
             .replace(" ", "")
             .split(",")
    )

    if len(color) != 3:
        raise ValueError("Color has to contain 3 elemets")

    return [to_u8(i) for i in color]

def to16(value):
    return int(value)&0xFF, int(value)>>8

def get_device():
    import hid

    devices = [i for i in hid.enumerate() if i["usage_page"] == USAGE_PAGE and i["usage"] == USAGE]

    if not devices:
        print("No devices found, quitting")
        sys.exit(0)

    if len(devices) == 1:
        return QP_XAP(hid.Device(path=devices[0]["path"]))

    names = [f"{i['manufacturer_string']}, {i['product_string']}" for i in devices]
    device_selector = [
        inquirer.List(
            "device",
            message="Select a device",
            choices=names,
            carousel=True,
        ),
    ]
    name = inquirer.prompt(device_selector, theme=BlueComposure())["device"]
    device = devices[names.index(name)]
    return QP_XAP(hid.Device(path=device["path"]))


def parse_bool(x):
    return str(x).lower() in ["true", "yes", "y", "1"]

def validate_color(_, x):
    try:
        parse_color(x)
        return True
    except:
        return False

def validate_number(_, x):
    try:
        int(x)
        return True
    except:
        return False

def validate_str(_, x):
    return True
# =================================================================================================


# ===================================== HID Abstraction =====================================
class QP_XAP:
    def __init__(self, hid):
        self._hid = hid
        print(f"Connected to {hid.manufacturer}:{hid.product}")

        # Empty buffer
        self._payload = [0x00] * 64

        # Set arbitrary token
        self._payload[0] = 0x42
        self._payload[1] = 0x42

    def _send(self, payload):
        if len(payload) > 61:
            raise ValueError(f"Payload is too long, ({len(payload)} > 61)")

        # Clear
        for i in range(2, len(self._payload)):
            self._payload[i] = 0x00

        # Copy payload
        self._payload[2] = len(payload)
        for i, byte in enumerate(payload, 3):
            self._payload[i] = byte

        # Windows needs an extra heading 0 byte
        if sys.platform == "win32":
            self._payload = [0, *self._payload]

        # Send
        self._hid.write(bytes(self._payload[:65])) #make sure we only send 64B
        response = self._hid.read(64, timeout=1000)

    def _close(self):
        self._hid.close()

    def clear(self, display):
        #dooesn't seem to do anything
        self._send([
            2, 2, 1,
            to_u8(display),
        ])

        #make all screen black
        self.rect(
            display=display,
            left=0,
            top=0,
            right=SCREEN_WIDTH,
            bottom=SCREEN_HEIGHT,
            color="0, 0, 0",
            filled=True,
        )

    def setpixel(self, *, display, x, y, color):
        self._send([
            2, 2, 2,
            to_u8(display),
            *to16(x),
            *to16(y),
            *parse_color(color),
        ])

    def line(self, *, display, x0, y0, x1, y1, color):
        self._send([
            2, 2, 3,
            to_u8(display),
            *to16(x0),
            *to16(y0),
            *to16(x1),
            *to16(y1),
            *parse_color(color),
        ])

    def rect(self, *, display, left, top, right, bottom, color, filled):
        filled = parse_bool(filled)

        self._send([
            2, 2, 4,
            to_u8(display),
            *to16(left),
            *to16(top),
            *to16(right),
            *to16(bottom),
            *parse_color(color),
            to_u8(filled),
        ])

    def circle(self, *, display, x, y, radius, color, filled):
        filled = parse_bool(filled)

        self._send([
            2, 2, 5,
            to_u8(display),
            *to16(x),
            *to16(y),
            *to16(radius),
            *parse_color(color),
            to_u8(filled),
        ])

    def ellipse(self, *, display, x, y, sizex, sizey, color, filled):
        filled = parse_bool(filled)

        self._send([
            2, 2, 6,
            to_u8(display),
            *to16(x),
            *to16(y),
            *to16(sizex),
            *to16(sizey),
            *parse_color(color),
            to_u8(filled),
        ])

    def drawimage(self, *, display, x, y, img):
        self._send([
            2, 2, 7,
            to_u8(display),
            *to16(x),
            *to16(y),
            to_u8(img),
        ])

    def drawimage_recolor(self, *, display, x, y, img, fg_color, bg_color):
        self._send([
            2, 2, 8,
            to_u8(display),
            *to16(x),
            *to16(y),
            to_u8(img),
            *parse_color(fg_color),
            *parse_color(bg_color),
        ])

    def animate(self, *, display, x, y, image):
        self._send([
            2, 2, 9,
            to_u8(display),
            *to16(x),
            *to16(y),
            to_u8(img),
        ])

    def animate_recolor(self, *, display, x, y, image, fg_color, bg_color):
        self._send([
            2, 2, 0xA,
            to_u8(display),
            *to16(x),
            *to16(y),
            to_u8(img),
            *parse_color(fg_color),
            *parse_color(bg_color),
        ])

    def drawtext(self, *, display, x, y, font, text):
        self._send([
            2, 2, 0xB,
            to_u8(display),
            *to16(x),
            *to16(y),
            to_u8(font),
            *[ord(i) for i in text],
        ])

    def drawtext_recolor(self, *, display, x, y, font, fg_color, bg_color, text):
        self._send([
            2, 2, 0xC,
            to_u8(display),
            *to16(x),
            *to16(y),
            to_u8(font),
            *parse_color(fg_color),
            *parse_color(bg_color),
            *[ord(i) for i in text],
        ])
# ===========================================================================================


# =========================== ASCII Art ===========================
TEXT = r"""
    ██╗    ██╗███████╗██╗      ██████╗ ██████╗ ███╗   ███╗███████╗
    ██║    ██║██╔════╝██║     ██╔════╝██╔═══██╗████╗ ████║██╔════╝
    ██║ █╗ ██║█████╗  ██║     ██║     ██║   ██║██╔████╔██║█████╗
    ██║███╗██║██╔══╝  ██║     ██║     ██║   ██║██║╚██╔╝██║██╔══╝
    ╚███╔███╔╝███████╗███████╗╚██████╗╚██████╔╝██║ ╚═╝ ██║███████╗
     ╚══╝╚══╝ ╚══════╝╚══════╝ ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝
 """

LOGO = r"""
          //   //  &/   //   /(
          //   //  ///  //   //
      %%%%%%%%%%%%%%%%%%%%%%%%%%%%&
     %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
////(%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%////
     %%%%%%   %%%%%   %%%%%  ,%%%%%%
////(%%%%%%   %%%%%   %%%%%  ,%%%%%%////
     %%%%%%   %%%%%   %%%%%  ,%%%%%%
////(%%%%%%.  %%%%%   %%%%%  %%%%%%%////
     %%%%%%%    %%%   %%%   %%%%%%%%
////(%%%%%%%%%%*         %%%%%%%%%%%////
     %%%%%%%%%%%%%%   %%%%%%%%%%%%%%
(///(%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%////
     %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
       %%%%%%%%%%%%%%%%%%%%%%%%%%%
          //   //  ///  //   //
"""
# =================================================================



if __name__ == "__main__":
    try:
        check_dependencies()

        global inquirer, BlueComposure
        import inquirer
        from inquirer.themes import BlueComposure


        import random

        # ==== Get XAP device
        qp_xap = get_device()


        # ==================== Print start message ====================
        text = TEXT.split("\n")
        text_rows = len(text)
        text_cols = max([len(row) for row in text])

        logo = LOGO.split("\n")
        logo_rows = len(logo)
        logo_cols = max([len(row) for row in logo])

        offset = (logo_rows - text_rows) // 2
        space  = (text_cols + 10)

        print("╸"*int((space+logo_cols)/1))
        for i, l in enumerate(logo):
            j = i - offset
            t = text[j] if j in range(text_rows) else ""
            spacer = " " * (space - len(t))
            print(f"{t}{spacer}{l}")

        print("╸"*int((space+logo_cols)/1))
        # =============================================================


        # ================== Prepare variables for the mainloop ==================
        methods = list(filter(lambda x: not x.startswith("_"), vars(QP_XAP).keys()))
        arguments = {
            method: [i for i in getattr(QP_XAP, method).__code__.co_varnames
                if i not in ["self", "log"]]
                for method in methods
        }

        prompts = {}
        for method in methods:
            prompt = []
            for arg in arguments.get(method):
                if arg in ["display", "font", "img"]:
                    message  = f"Select a{'n' if arg == 'image' else ''} {arg}"
                    validate = validate_number
                    default  = "0"

                elif "color" in arg:
                    message  = f"Input {'a ' if arg =='color' else ''} {arg}"
                    validate = validate_color
                    default  = lambda _: f"[{random.randint(0, 255)}, 255, 255]"

                elif arg == "filled":
                    prompt.append(inquirer.List(
                        arg,
                        message="Want it filled?",
                        choices=["Yes", "No"],
                        default="Yes",
                        carousel=True,
                    ))
                    continue

                elif arg == "text":
                    message  = "Write something"
                    validate = validate_str
                    default  = "Hello!"

                else:
                    message  = f"Input a value for {arg}"
                    validate = validate_number
                    default  = lambda _: f"{random.randint(0, 100)}"

                prompt.append(inquirer.Text(
                    arg,
                    message=message,
                    validate=validate,
                    default=default,
                ))

            prompts[method] = prompt

        methods.append("quit")
        method_selector = [
            inquirer.List(
                "method",
                message="What do you want to do?",
                choices=methods,
                carousel=True,
            ),
        ]
        # ========================================================================


        # ============================ Mainloop ============================
        while True:
            method = inquirer.prompt(method_selector, theme=BlueComposure())
            # not using walrus (:=) so this is compatible with older version
            if method is None or method["method"] == "quit":
                break

            method = method["method"]
            args = inquirer.prompt(prompts[method], theme=BlueComposure())
            # Call the function
            getattr(qp_xap, method)(**args)
        # ==================================================================

    except Exception as e:
        print(f"Quitting due to [{e.__class__.__name__}]: {e}")
        qp_xap._close()
