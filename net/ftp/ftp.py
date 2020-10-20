import socket
import re
import os
import shutil
import sys

buffer_size = 1024


def exe_cmd(_sock, cmd):
    """
    execute command line
    :param _sock: socket fd
    :param cmd: command string
    :return: 0 success otherwise fail
    """
    cmd = bytes(cmd + "\r\n", encoding="utf-8")
    try:
        _sock.send(cmd)
    except Exception as e:
        print("error occur", e)
        return -1, ""
    return 0


def connect(host, port):
    """
    create connection
    :param host: server address
    :param port:  server port
    :return: socket fd
    """
    _sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    _sock.connect((host, port))
    return _sock


def get_decode_host_port(_sock):
    """
    cal ip port from server
    :param _sock: socket fd
    :return: ip, port
    """
    buf = b''
    while True:
        buf += _sock.recv(buffer_size)
        if b'Entering Passive Mode' in buf:
            break

    resp = buf.decode("utf-8")

    for line in resp.splitlines():
        nums = re.findall(r"\d+", line[10:])
        if len(nums) == 6:
            ip = ".".join(nums[:4])
            port = (int(nums[4]) << 8) + int(nums[5])
            return ip, port
    return "", -1


def login(_sock, user, password):
    """
    login server
    :param _sock:
    :return: 0 success otherwise fail

    """
    exe_cmd(_sock, "USER " + user)
    resp = _sock.recv(buffer_size)
    exe_cmd(_sock, "PASS " + password)
    resp = _sock.recv(buffer_size)
    # if resp.startswith(b"2"):
    #     return 0
    # return -1


def ls(_sock, path):
    """
    ls command
    :param _sock:
    :param path:
    :return:
    """
    exe_cmd(_sock, "PASV")
    ip, port = get_decode_host_port(_sock)

    if ip == "":
        print("error occur\n")
        return

    data_socket = connect(ip, port)
    exe_cmd(_sock, "LIST " + path)
    resp = data_socket.recv(buffer_size)
    resp = resp.decode("utf-8")
    print(resp)
    data_socket.close()

    # _sock.close()


def mkdir(_sock, path):
    """
    mkdir
    :param _sock:
    :param path:
    :return:
    """
    exe_cmd(_sock, "MKD " + path)
    resp = _sock.recv(buffer_size)
    resp = resp.decode("utf-8")[3:]
    print(resp)


def rm(_sock, path):
    """
    remove file
    :param _sock: socket fd
    :param path: file path
    :return:
    """
    exe_cmd(_sock, "DELE " + path)
    resp = _sock.recv(buffer_size)
    resp = resp.decode("utf-8")[4:]
    print(resp)


def rmdir(_sock, path):
    """
    remove dir
    :param _sock:
    :param path:
    :return:
    """
    exe_cmd(_sock, "RMD " + path)
    resp = _sock.recv(buffer_size)
    resp = resp.decode("utf-8")[4:]
    print(resp)


def upload_data(_sock, path, data):
    """
    upload data to server
    :param _sock: socket fd
    :param path: server path
    :param data: data
    :return:
    """
    exe_cmd(_sock, "PASV")
    ip, port = get_decode_host_port(_sock)
    data_socket = connect(ip, port)
    exe_cmd(_sock, "STOR " + path)
    resp = _sock.recv(buffer_size)

    for d in data:
        data_socket.send(d)
    # resp = _sock.recv(buffer_size)
    print(resp)
    data_socket.close()


def download_data(_sock, path):
    """
    download data
    :param _sock: socket fd
    :param path: server path
    :return:
    """
    exe_cmd(_sock, "PASV")
    ip, port = get_decode_host_port(_sock)
    data_socket = connect(ip, port)
    exe_cmd(_sock, "RETR " + path)

    local_path = "/tmp/" + "download.tmp"
    local_f = open(local_path, "wb")
    while True:
        resp = data_socket.recv(1024)
        local_f.write(resp)
        if resp == b"":
            break
    local_f.flush()
    local_f.close()
    data_socket.close()
    print("download ok...")
    return local_path


def cp(_sock, path1, path2, flag=0):
    """
    copy path1 to path2
    :param _sock:
    :param path1:
    :param path2:
    :param flag: 0 local to server, 1 server to local , 2 server to server
    :return:
    """
    if flag == 0:
        if not os.path.exists(path1):
            print("not exist local file")
            return
        wf = open(path1, "rb")
        upload_data(_sock, path2, wf)
        wf.close()
    elif flag == 1:
        local_path = download_data(_sock, path1)
        shutil.move(local_path, path2)
    elif flag == 2:
        local_path = download_data(_sock, path1)
        wf = open(local_path, "rb")
        upload_data(_sock, path2, wf)


def mv(_sock, path1, path2, flag=0):
    """
    move path1 to path2
    :param _sock:  as same above
    :param path1:
    :param path2:
    :param flag:
    :return:
    """
    cp(_sock, path1, path2, flag)
    if flag == 0:
        os.remove(path1)
    elif flag == 1 or flag == 2:
        rm(_sock, path1)


def parser_url(url):
    """
    parser url
    :param url:
    :return: 0, local file, 1 server file , -1 error
    """
    if os.path.exists(url):
        return 0, url

    res = re.findall(r"ftp://(\w+):(\w+)@(.+?)/(.*)", url)
    # print(res)
    if len(res) != 1:
        return -1, ""
    user, password, host, path = res[0]
    return 1, (user, password, host, path)


def show_usage():
    """
    show help
    :return:
    """
    print(" --help, -h, show help..")
    print(" support command:")
    print(" ls <path-to-file>")
    print(" rm <path-to-file>")
    print(" rmd <path-to-dir>")
    print(" mkdir <path-to-dir>")
    print(" cp ARG1 ARG2")
    print(" mv ARG1 ARG2")


def parser_command_line():
    """
    parser command line
    :return:
    """
    argv = sys.argv
    if len(argv) < 2:
        show_usage()
        return
    cmd = argv[1]
    if cmd == "--help" or cmd == "-h":
        show_usage()
        return
    elif cmd == "ls" or cmd == "rm" or cmd == "rmdir" or cmd == "mkdir":
        if len(argv) < 3:
            print("expected path")
            return
        r, d = parser_url(argv[2])
        if r != 1:
            print("url error")
            return
        username, password, host, path = d
        sock = connect(host, 21)
        login(sock, username, password)

        if cmd == "ls": ls(sock, path)
        elif cmd == "rm": rm(sock, path)
        elif cmd == "rmdir": rmdir(sock, path)
        elif cmd == "mkdir": mkdir(sock, path)
        exe_cmd(sock, "QUIT")
        sock.close()
    elif cmd == "cp" or cmd == "mv":
        if len(argv) < 4:
            print("expected two path")
            return
        ret1, p1 = parser_url(argv[2])
        ret2, p2 = parser_url(argv[3])
        if ret1 == -1 or ret2 == -1:
            print("path error")
            return
        flag = -1
        username, password, host, path2, path1 = "", "", "", "", ""

        if ret1 == 0 and ret2 == 1:
            flag = 0
            username, password, host, path2 = p2
            path1 = p1

        if ret1 == 1 and ret2 == 0:
            flag = 1
            username, password, host, path1 = p1
            path2 = p2

        if ret1 == 1 and ret2 == 1:
            flag = 2
            username, password, host, path1 = p1
            username, password, host, path2 = p2

        if flag == -1:
            print("path error")
            return
        sock = connect(host, 21)
        login(sock, username, password)

        if cmd == "cp":
            cp(sock, path1, path2, flag)
        elif cmd == "mv":
            mv(sock, path1, path2, flag)


if __name__ == '__main__':
    # username = "maoyue"
    # password = "LRufsOZdj3U7tWMDVcFQ"
    # _s = connect("3700.network", 21)
    # login(_s, username, password)
    # mkdir(_s, "/my_test")
    # ls(_s, r"/")
    # rm(_s, "/hello.txt")
    # ls(_s, "/")
    # rmdir(_s, "/my_test")
    # ls(_s, "/")
    # upload_data(_s, "/test.txt", b"wo shi test" * 1000)
    # download_data(_s, "/test.txt")

    # mv(_s, "/read_pacp.py", "/my_stuff/tmp_read_pacp.py", 2)
    # exe_cmd(_s, "QUIT")
    # _s.close()
    # print(parser_url("ftp://maoyue:LRufsOZdj3U7tWMDVcFQ@3700.network/test"))
    parser_command_line()
