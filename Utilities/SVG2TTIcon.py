#!/usr/bin/env python3

from optparse import OptionParser
import xml.etree.ElementTree as ET
import struct

def float_to_fixedPoint1_13(x, flag):
    """Convert a floating point number between -2.0 and 1.99 to an 15 bit signed integer"""
    r = int(x * 16384.0)
    if (r < -32768 or r > 32767):
        raise OverflowError("%f is not within -2.0 and 1.9998779296875" % (x))

    r &= 0xfffe
    if flag:
        r |= 1

    return struct.pack("<H", r)



color_names = {
    "black": "#000000",
    "green": "#008000",
    "silver": "#C0C0C0",
    "lime": "#00FF00", 
    "gray": "#808080",
    "olive": "#808000", 
    "white": "#FFFFFF",
    "yellow": "#FFFF00", 
    "maroon": "#800000",
    "navy": "#000080", 
    "red": "#FF0000",
    "blue": "#0000FF", 
    "purple": "#800080",
    "teal": "#008080", 
    "fuchsia": "#FF00FF",
    "aqua": "#00FFFF"
}

def gammaToLinear(u):
    if u <= 0.04045:
        return u / 12.92
    else:
        return ((u + 0.055) / 1.055) ** 2.4

def parseHexColor(s):
    h = s[1:].lower()
    if len(h) == 6:
        h += "ff"

    if len(h) == 3:
        h += "f"

    if len(h) == 4:
        h = h[0]+h[0] + h[1]+h[1] + h[2]+h[2] + h[3]+h[3] 

    if len(h) != 8:
        raise RuntimeError("Expect hex color '%s' format with 6 or 8 hex digits." % s)

    return (
        gammaToLinear(int(h[0:2], 16) / 255.0),
        gammaToLinear(int(h[2:4], 16) / 255.0),
        gammaToLinear(int(h[4:6], 16) / 255.0),
        int(h[6:8], 16) / 255.0
    )

class Color (object):
    def __init__(self, s=None):
        # Color is stored as scRGBA. 
        if s:
            s = color_names.get(s.lower(), s)
            if s.startswith("#"):
                self.color = parseHexColor(s)
            else:
                raise RuntimeError("Unexpect color format '%s'" % s)
        else:
            self.color = (0.0, 0.0, 0.0, 0.0)

    def __repr__(self):
        return "<Color %f, %f, %f, %f>" % self.color

    def serialize(self):
        r =	self.redInt16()
        g = self.greenInt16()
        b = self.blueInt16()
        a = self.alphaInt16()
        return struct.pack("<HHHH", r, g, b, a)

    def redInt16(self):
        x = int(self.color[0] * 8192.0 + 4096.0)
        if x < -32768 or x > 32767:
            raise OverflowError("Color component value %f should be between -0.5 and 7.5" % self.color[0])
        return x
        
    def greenInt16(self):
        x = int(self.color[1] * 8192.0 + 4096.0)
        if x < -32768 or x > 32767:
            raise OverflowError("Color component value %f should be between -0.5 and 7.5" % self.color[1])
        return x

    def blueInt16(self):
        x = int(self.color[2] * 8192.0 + 4096.0)
        if x < -32768 or x > 32767:
            raise OverflowError("Color component value %f should be between -0.5 and 7.5" % self.color[2])
        return x

    def alphaInt16(self):
        x = int(self.color[3] * 65535.0)
        if x < 0 or x > 65535:
            raise OverflowError("Alpha value %f should be between 0.0 and 1.0" % self.color[3])
        return x
    

class Point (object):
    ANCHOR = 0
    CUBIC_CONTROL1 = 1
    CUBIC_CONTROL2 = 2
    QUADRATIC_CONTROL = 3

    def __init__(self, x, y, kind):
        self.x = x
        self.y = y
        self.kind = kind

    def __repr__(self):
        return "<Point %f, %f, %i>" % (self.x, self.y, self.kind)

    def serialize(self):
        x_flag = (self.kind == Point.CUBIC_CONTROL1) or (self.kind == Point.QUADRATIC_CONTROL)
        y_flag = (self.kind == Point.CUBIC_CONTROL2) or (self.kind == Point.QUADRATIC_CONTROL)
        return float_to_fixedPoint1_13(self.x, x_flag) + float_to_fixedPoint1_13(self.y, y_flag)	

    def transform(self, xOffset, yOffset, widthScale, heightScale):
        self.x = (self.x + xOffset) * widthScale
        self.y = (self.y + yOffset) * heightScale


class Contour (object):
    def __init__(self):
        self.points = []

    def __repr__(self):
        return "<Contour %s>" % (self.points)

    def serialize(self):
        header = struct.pack("<H", len(self.points))
        body = b"".join(x.serialize() for x in self.points)
        return header + body

    def add(self, point):
        self.points.append(point)

    def addPoint(self, x, y, kind):
        self.points.append(Point(x, y, kind))

    def transform(self, xOffset, yOffset, widthScale, heightScale):
        for point in self.points:
            point.transform(xOffset, yOffset, widthScale, heightScale)



class Path (object):
    def __init__(self, fillColor, strokeColor, strokeWidth):
        self.fillColor = fillColor
        self.strokeColor = strokeColor
        self.strokeWidth = strokeWidth
        self.contours = []

    def __repr__(self):
        return "<Path color=%s %s>" % (self.color, self.contours)

    def serialize(self):
        header = self.fillColor.serialize()
        header += self.strokeColor.serialize()
        # False flag here means that the line join style is Miter, otherwise it is Bevel.
        header += float_to_fixedPoint1_13(self.strokeWidth, False)
        header += struct.pack("<H", len(self.contours))
        body = b"".join(x.serialize() for x in self.contours)
        return header + body

    def add(self, contour):
        self.contours.append(contour)

    def transform(self, xOffset, yOffset, widthScale, heightScale):
        self.strokeWidth *= widthScale
        for contour in self.contours:
            contour.transform(xOffset, yOffset, widthScale, heightScale)

class Icon (object):
    def __init__(self):
        self.boundingBox = (0.0, 0.0, 1.0, 1.0)
        self.paths = []
        self.title = None

    def __repr__(self):
        return "<Icon bb=%s %s>" % (self.boundingBox, self.paths)

    def serialize(self):
        header = b"TTIC"
        header += struct.pack(
            "<H",
            len(self.paths)
        )
        body = b"".join(x.serialize() for x in self.paths)
        return header + body

    def add(self, path):
        self.paths.append(path)

    def setBoundingBox(self, x, y, width, height):
        self.boundingBox = (x, y, width, height)

    def addRectangle(self, x, y, width, height, fillColor, strokeColor, strokeWidth):
        path = Path(fillColor, strokeColor, strokeWidth)
        # Clockwise, because y-axis is flipped.'
        contour = Contour()
        contour.addPoint(x, y, Point.ANCHOR)
        contour.addPoint(x+width, y, Point.ANCHOR)
        contour.addPoint(x+width, y+height, Point.ANCHOR)
        contour.addPoint(x, y+height, Point.ANCHOR)
        path.add(contour)
        self.add(path)

    def normalize(self):
        xOffset = -self.boundingBox[0] - (self.boundingBox[2] * 0.5)
        yOffset = -self.boundingBox[1] - (self.boundingBox[3] * 0.5)
        widthScale = 4.0 / self.boundingBox[2]
        heightScale = -4.0 / self.boundingBox[3]

        for path in self.paths:
            path.transform(xOffset, yOffset, widthScale, heightScale)

        self.boundingBox = (
            (self.boundingBox[0] + xOffset) * widthScale,
            -(self.boundingBox[1] + yOffset) * heightScale,
            self.boundingBox[2] * widthScale,
            -self.boundingBox[3] * heightScale
        )

def splitPathTokens(s):
    float_token = ""
    for c in s:
        if c in "0123456789-.":
            float_token += c
        elif c == " ":
            if float_token:
                yield float(float_token)
                float_token = ""
        else:
            if float_token:
                yield float(float_token)
            float_token = ""
            yield c

    if float_token:
        yield float_token


def convertContours(s):
    path_tokens = [x for x in splitPathTokens(s)]

    contours = []
    contour = None
    i = 0
    while i < len(path_tokens):
        command = path_tokens[i]
        if command == "M":
            contour = Contour()
            contour.addPoint(path_tokens[i+1], path_tokens[i+2], Point.ANCHOR)
            i += 3
        elif command == "L":
            contour.addPoint(path_tokens[i+1], path_tokens[i+2], Point.ANCHOR)
            i += 3
        elif command == "C":
            contour.addPoint(path_tokens[i+1], path_tokens[i+2], Point.CUBIC_CONTROL1)
            contour.addPoint(path_tokens[i+3], path_tokens[i+4], Point.CUBIC_CONTROL2)
            contour.addPoint(path_tokens[i+5], path_tokens[i+6], Point.ANCHOR)
            i += 7
        elif command == "S":
            contour.addPoint(path_tokens[i+1], path_tokens[i+2], Point.CUBIC_CONTROL2)
            contour.addPoint(path_tokens[i+3], path_tokens[i+4], Point.ANCHOR)
            i += 5
        elif command == "Q":
            contour.addPoint(path_tokens[i+1], path_tokens[i+2], Point.QUADRATIC_CONTROL)
            contour.addPoint(path_tokens[i+3], path_tokens[i+4], Point.ANCHOR)
            i += 5
        elif command in "Zz":
            contours.append(contour)
            contour = None
            i += 1
        else:
            raise NotImplementedError("Does not implement '%s' command" % command)
    
    return contours

def parseSVGIconGroup(icon, group):
    id = group.attrib["id"]
    title = id

    for child in group:
        if child.tag == "{http://www.w3.org/2000/svg}title":
            title = child.text

        elif child.tag == "{http://www.w3.org/2000/svg}g":
            parseSVGIconGroup(icon, child)

        elif child.tag == "{http://www.w3.org/2000/svg}path":
            if not title.startswith("__"):
                fillColor = Color(child.attrib.get("fill", None))
                strokeColor = Color(child.attrib.get("stroke", None))
                strokeWidth = float(child.attrib.get("stroke-width", 1.0))

                path = Path(fillColor, strokeColor, strokeWidth)
                contours = convertContours(child.attrib["d"])
                for contour in contours:
                    path.add(contour)

                icon.add(path)

        elif child.tag == "{http://www.w3.org/2000/svg}rect":
            if title == "__TTauriIconBounds":
                icon.setBoundingBox(
                    float(child.attrib["x"]),
                    float(child.attrib["y"]),
                    float(child.attrib["width"]),
                    float(child.attrib["height"])
                )

            elif title.startswith("__"):
                # Ignore anying in layers starting with a double underscore.
                pass

            else:
                icon.addRectangle(
                    float(child.attrib["x"]),
                    float(child.attrib["y"]),
                    float(child.attrib["width"]),
                    float(child.attrib["height"]),
                    Color(child.attrib.get("fill", None)),
                    Color(child.attrib.get("stroke", None)),
                    float(child.attrib.get("stroke-width", 1.0))
                )

    # Set the icon title to the title of the top level group
    icon.title = title


def parseSVGIcon(filename):
    tree = ET.parse(filename)
    root = tree.getroot()

    icon = Icon()
    for child in root:
        if child.tag == "{http://www.w3.org/2000/svg}g":
            parseSVGIconGroup(icon, child)
    return icon

def main():
    parser = OptionParser()
    parser.add_option(
        "-o", "--output",
        dest="output_filename", metavar="FILE", default=None,
        help="The filename of the TTauri Icon file, recomended extension is .tticons"
    )
    parser.add_option(
        "-v", "--verbose",
        action="store_true", dest="verbose", default=False,
        help="Display verbose messages."
    )
    (options, input_filenames) = parser.parse_args()

    if len(input_filenames) != 1:
        print("Expecting exactly 1 input file.", file=sys.stderr)
        sys.exit(2)

    input_filename = input_filenames[0]

    if options.verbose:
        print("Parsing '%s'" % (input_filename))

    icon = parseSVGIcon(input_filename)
    icon.normalize()

    if options.output_filename:
        if options.verbose:
            print("Writing icon to '%s'" % (options.output_filename,), file=sys.stderr)

        fd = open(options.output_filename, "wb")
        fd.write(icon.serialize())
        fd.close()


if __name__ == "__main__":
    main()
