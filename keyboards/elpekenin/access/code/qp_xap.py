#!/usr/bin/env python3

"""This script provides a (somewhat) friendly interface to interact with your Quantum Painter
devices over a XAP connection.

Note: Has only been tested on 3.10.8 and 3.11, but should work down to 3.6 (f-strings)
"""
import importlib
import logging
import random
import subprocess
import sys


# ======= CONFIG =======
USAGE = 0x0058
USAGE_PAGE = 0xFF51
SCREEN_WIDTH = 480
SCREEN_HEIGHT = 320
LOG_LEVEL = logging.CRITICAL
# ======================


logging.basicConfig(
    format="%(levelname)s - %(message)s",
    level=LOG_LEVEL,
    # filename="qp_xap.log",
    # filemode="w",
)


# ============================================ HELPERS ============================================
def log_exception(exception):
    """Log the type of exception that happened
    """
    logging.error("Exception was handled: [%s]%s", exception.__class__.__name__, exception)


def check_dependencies():
    """Checks for the needed dependencies. If any is missing, user is prompted get it installed.
    """
    for dep in ["hid", "inquirer"]:
        try:
            importlib.import_module(dep)  # pylint: disable=import-outside-toplevel
            logging.info("%s was already installed ✔", dep)

        except ModuleNotFoundError as imp_exc:
            log_exception(imp_exc)
            ans = input(f"Module {dep} not installed, do you want to install it? [Y/n]: ")
            if ans.lower() not in ["", "y", "yes"]:
                print("Can't procceed")

            print("-------------")
            logging.info("Intalling %s through pip", dep)
            subprocess.run([sys.executable, "-m", "pip", "install", dep], check=True)
            print("-------------")


def get_device():
    """Finds compatible endpoints:
        - If none: Quits
        - In one: Automatically select it
        - If many: Prompt user to select one
    """
    import hid  # pylint: disable=import-outside-toplevel
    import inquirer  # pylint: disable=import-outside-toplevel
    from inquirer.themes import BlueComposure  # pylint: disable=import-outside-toplevel

    devices = [i for i in hid.enumerate() if i["usage_page"] == USAGE_PAGE and i["usage"] == USAGE]

    if not devices:
        print("No devices found, quitting")
        sys.exit(0)

    if len(devices) == 1:
        return hid.Device(path=devices[0]["path"])

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
    return hid.Device(path=device["path"])


def to_u8(value):
    """Convert any number to 8 bit representation (can lose some of MSB)
    """
    return int(value) & 0xFF


def to16(value):
    """Convert any number to 16 bit representation (can lose some of MSB)
    """
    return int(value) & 0xFF, int(value) >> 8


def parse_bool(value):
    """Parse string into boolean
    """
    return str(value).lower() in ["true", "yes", "y", "1"]


def generate_color(_):
    """Generate a color
        - H = random(0-255)
        - S = 255
        - V = 255
    """
    return f"[{random.randint(0, 255)}, 255, 255]"


def parse_color(color):
    """Parse string into color
    """
    color = (
        color.replace("[", "")
             .replace("]", "")
             .replace(" ", "")
             .split(",")
    )

    if len(color) != 3:
        raise ValueError("Color has to contain 3 elemets")

    return [to_u8(i) for i in color]


def validate_color(_, value):
    """Validate if string is a color
    """
    try:
        parse_color(value)
        return True
    except ValueError:
        return False


def generate_number(_):
    """Generate a number = random(0, 100)
    """
    return f"{random.randint(0, 100)}"


def validate_number(_, value):
    """Validate if string is a number
    """
    try:
        int(value)
        return True
    except ValueError:
        return False


def validate_str(__, _):
    """Validate if string has correct content
    """
    return True


# ===================================== HID Abstraction =====================================
class QpXap:
    """Abstraction over HID details, to easily call any of the QP functions over XAP
    """
    def __init__(self, dev):
        self._hid = dev
        print(f"Connected to {dev.manufacturer}:{dev.product}")

        # Empty buffer
        self._payload = [0x00] * 64

        # Set arbitrary token
        self._payload[0] = 0x42
        self._payload[1] = 0x42

    def _send(self, payload):
        if len(payload) > 61:
            raise ValueError(f"Payload is too long, ({len(payload)} > 61)")

        # Clear
        for i in range(2, len(self._payload)):  # pylint: disable=redefined-outer-name
            self._payload[i] = 0x00

        # Copy payload
        self._payload[2] = len(payload)
        for i, byte in enumerate(payload, 3):
            self._payload[i] = byte

        # Windows needs an extra heading 0 byte
        if sys.platform == "win32":
            self._payload = [0, *self._payload]

        # Send
        self._hid.write(bytes(self._payload[:65]))  # make sure we only send 64B
        _ = self._hid.read(64, timeout=1000)

    def _close(self):
        self._hid.close()

    def __del__(self):
        self._close()

    def clear(self, display):
        """Fill the selected display black
        """
        # dooesn't seem to do anything
        self._send([
            2, 2, 1,
            to_u8(display),
        ])

        # make all screen black
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
        """Draw a single pixel
        """
        self._send([
            2, 2, 2,
            to_u8(display),
            *to16(x),
            *to16(y),
            *parse_color(color),
        ])

    def line(self, *, display, x0, y0, x1, y1, color):
        """Draw a line
        """
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
        """Draw a rectangle
        """
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
        """Draw a circle
        """
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
        """Draw an ellipse
        """
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
        """Draw an image (based on its array index)
        """
        self._send([
            2, 2, 7,
            to_u8(display),
            *to16(x),
            *to16(y),
            to_u8(img),
        ])

    def drawimage_recolor(self, *, display, x, y, img, fg_color, bg_color):
        """Draw an image (based on its array index) and change its colors
        """
        self._send([
            2, 2, 8,
            to_u8(display),
            *to16(x),
            *to16(y),
            to_u8(img),
            *parse_color(fg_color),
            *parse_color(bg_color),
        ])

    def animate(self, *, display, x, y, img):
        """Draw an animation (based on its array index)
        """
        self._send([
            2, 2, 9,
            to_u8(display),
            *to16(x),
            *to16(y),
            to_u8(img),
        ])

    def animate_recolor(self, *, display, x, y, img, fg_color, bg_color):
        """Draw an animation (based on its array index) and change its colors
        """
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
        """Write some text
        """
        self._send([
            2, 2, 0xB,
            to_u8(display),
            *to16(x),
            *to16(y),
            to_u8(font),
            *[ord(i) for i in text],
        ])

    def drawtext_recolor(self, *, display, x, y, font, fg_color, bg_color, text):
        """Write some text and change its colors
        """
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


def welcome_message():
    """Show a greeting after checking dependencies and connecting to a device
    """
    text_rows = TEXT.split("\n")
    n_text_rows = len(text_rows)
    n_text_cols = max(len(row) for row in text_rows)

    logo_rows = LOGO.split("\n")
    n_logo_rows = len(logo_rows)
    n_logo_cols = max(len(row) for row in logo_rows)

    offset = (n_logo_rows - n_text_rows) // 2
    space = (n_text_cols + 10)

    print("╸" * int((space + n_logo_cols) / 1))
    # This code assumes logo has more rows than text does
    for logo_index, logo_row in enumerate(logo_rows):
        text_index = logo_index - offset
        text_row = text_rows[text_index] if text_index in range(n_text_rows) else ""
        spacer = " " * (space - len(text_row))
        print(f"{text_row}{spacer}{logo_row}")

    print("╸" * int((space + n_logo_cols) / 1))


# =========================== Main loop ===========================
def user_input_loop(qp_xap):
    """Listen to user input and send messages to the connected device
    """
    import inquirer  # pylint: disable=import-outside-toplevel
    from inquirer.themes import BlueComposure  # pylint: disable=import-outside-toplevel

    methods = list(filter(lambda x: not x.startswith("_"), vars(QpXap).keys()))
    arguments = {
        method: [
            i
            for i in getattr(QpXap, method).__code__.co_varnames
            if i not in ["self", "log"]
        ]
        for method in methods
    }

    prompts = {}
    for method in methods:
        prompt = []
        for arg in arguments.get(method):
            if arg in ["display", "font", "img"]:
                message = f"Select a{'n' if arg == 'img' else ''} {arg}"
                validate = validate_number
                default = "0"

            elif "color" in arg:
                message = f"Input {'a ' if arg =='color' else ''} {arg}"
                validate = validate_color
                default = generate_color

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
                message = "Write something"
                validate = validate_str
                default = "Hello!"

            else:
                message = f"Input a value for {arg}"
                validate = validate_number
                default = generate_number

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

    while True:
        # Get method
        method = inquirer.prompt(method_selector, theme=BlueComposure())
        if method is None or method["method"] == "quit":
            break

        # Get args
        method = method["method"]
        args = inquirer.prompt(prompts[method], theme=BlueComposure())

        # Call the function
        getattr(qp_xap, method)(**args)


if __name__ == "__main__":
    try:
        check_dependencies()
        hid_dev = get_device()
        _qp_xap = QpXap(hid_dev)
        welcome_message()
        user_input_loop(_qp_xap)

    except Exception as run_exc:  # pylint: disable=broad-except
        log_exception(run_exc)

    finally:
        del _qp_xap
