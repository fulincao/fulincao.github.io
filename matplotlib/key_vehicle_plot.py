import os, sys
import json
import matplotlib.pyplot as plt


def run():


    x, y = [], []
    sp_y = []

    p_x = []
    p_y = []

    l_x = []
    l_y = []

    r_x = []
    r_y = []

    fig, ax = plt.subplots()

    ax.plot(x, y, marker=".", picker=5, label="veh_id")
    ax.plot(x, sp_y, marker=".", picker=5, label="veh_sp*1000")
    ax.plot(p_x, p_y, marker=".", picker=5, label="ped_id")
    ax.plot(l_x, l_y, marker=".", picker=5, label="left_lane")
    ax.plot(r_x, r_y, marker=".", picker=5, label="right_lane")

    # 滚轮放大缩小
    def call_back(event):
        axtemp = event.inaxes
        if not axtemp:
            return
        x_min, x_max = axtemp.get_xlim()
        y_min, y_max = axtemp.get_ylim()
        x_dt = (x_max - x_min) / 10
        y_dt = (y_max - y_min) / 10

        if event.button == 'up':
            ax.set(xlim=(x_min + x_dt, x_max - x_dt))
            ax.set(ylim=(y_min + y_dt, y_max - y_dt))
        elif event.button == 'down':
            ax.set(xlim=(x_min - x_dt, x_max + x_dt))
            ax.set(ylim=(y_min - y_dt, y_max + y_dt))
        elif event.button == 3:
            axtemp.scatter(event.xdata, event.ydata, s=10, color='black', marker=',')
            axtemp.annotate('{0:.3f},{1:.3f}'.format(event.xdata, event.ydata), xy=(event.xdata, event.ydata))
        fig.canvas.draw_idle()  # 绘图动作实时反映在图像上

    annotation = plt.annotate(('', ''), xy=(24500+ 0.1, 2800 + 0.1), xycoords='data',
                                xytext=(24500, 2800 + 0.7),
                                textcoords='data', horizontalalignment="left",
                                arrowprops=dict(arrowstyle="simple", connectionstyle="arc3,rad=-0.1"),
                                bbox=dict(boxstyle="round", facecolor="w", edgecolor="0.5", alpha=0.9)
                                )
    annotation.set_visible(False)

    ## 选择表示点信息
    def onpick(event):
        thisline = event.artist
        xdata = thisline.get_xdata()
        ydata = thisline.get_ydata()
        ind = event.ind
        points = tuple(zip(xdata[ind], ydata[ind]))
        print('onpick points:', points)
        if len(points) > 0:
            now_x, now_y = points[0][0], points[0][1]

            annotation.set_text(str(now_x) + "," + str(now_y))
            # annotation.set_xytext((str(now_x), str(now_y)))
            annotation.set_position((now_x+1, now_y+10))
            # annotation.set_x(now_x)
            # annotation.set_y(now_y)
            annotation.xy =(now_x, now_y)
            annotation.set_visible(True)
            plt.draw()

    # fig.canvas.mpl_connect('button_press_event', call_back)
    fig.canvas.mpl_connect('scroll_event', call_back)
    fig.canvas.mpl_connect("pick_event", onpick)

    plt.title("key_vehicle")
    plt.xlabel("frame_id")
    plt.legend()
    plt.grid()
    plt.show()
    plt.clf()


if __name__ == '__main__':
    sys.argv.append("/home/cao/work-git/pcview-v2/x1d3_view/pcview_data/20200916193423/20200916193423.txt")
    run()
    # pass