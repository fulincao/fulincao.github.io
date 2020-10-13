
class MpegWriter():
    def __init__(self, width, height, fps, video_path, fourcc="MJPG"):
        self.width = width
        self.height = height
        self.fps = fps
        self.fourcc = fourcc        
        self.vidoe_path = video_path
        self.fp = None

    def init(self):
        self.fp = open(self.vidoe_path, "wb")
        self.write_header()

    def write_header(self):
        pass
        
    def write(self, jpg):
        pass
    
    def write_index(self):
        pass

    def release(self):
        if self.fp is not None:
            self.write_index()
            self.fp.fluse()
            self.fp.close()


if __name__ == "__main__":
    pass