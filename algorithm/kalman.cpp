
#include <Eigen/Dense>
#include <iostream>


class Kalman {
    public:
        
        Kalman(){
            is_init = false;
        }

        ~Kalman(){
            ;
        }

        bool isInit(){
            return is_init;
        }

        // 初始化状态向量
        void init(Eigen::VectorXd x_in){
            x_ = x_in;
            is_init = true;
        }

        // 状态转移矩阵
        void setF(Eigen::MatrixXd F_in){
            F_ = F_in;
        }

        // 系统的不确定度
        void setP(Eigen::MatrixXd P_in){
            P_ = P_in;
        }

        // 过程噪声 （一般为单位矩阵）
        void setQ(Eigen::MatrixXd Q_in){
            Q_ = Q_in;
        }

        // 测量矩阵
        void setH(Eigen::MatrixXd H_in){
            H_ = H_in;
        }

         // 测量噪声矩阵（一般由传感器厂家提供）
        void setR(Eigen::MatrixXd R_in){
            R_ = R_in;
        }

        // 计算预测值
        void predict(){
            x_ = F_ * x_;
            // std::cout << x_ << std::endl;
            //printf("predict ok, x_ shape is %d %d\n", x_.rows(), x_.cols());
            P_ = F_*P_*F_.transpose();
            //printf("predict ok, P_ shape is %d %d\n", P_.rows(), P_.cols());
        }

        // 观测
        void measurement_update(const Eigen::VectorXd &z){
            // 观测值z与预测值x的差值y
            Eigen::VectorXd y = z - H_ * x_;
            //printf("update ok, y_ shape is %d %d\n", y.rows(), y.cols());
            // 求解卡尔曼增益k，差值y的权重
            Eigen::MatrixXd S_ = H_ * P_ * H_.transpose() + R_;
            //printf("update ok, S_ shape is %d %d\n", S_.rows(), S_.cols());
            Eigen::MatrixXd K_ = P_ * H_.transpose() * S_.inverse();
            //printf("update ok, K_ shape is %d %d\n", K_.rows(), K_.cols());
            // 更新状态向量，考虑了测量值，预测值，整个系统的噪声
            x_ = x_ + K_ * y;
            int size = x_.size();
            Eigen::MatrixXd I = Eigen::MatrixXd::Identity(size, size);
            P_ = (I - K_ * H_) * P_;
        }

        Eigen::VectorXd getX(){
            return x_;
        }

    private:
        bool is_init;
        Eigen::VectorXd x_;
        Eigen::MatrixXd F_, P_, Q_, H_, R_;
};


int main(int argc, char* argv[]){

    Kalman kalman;

    int iter=0;
    double mx = 1.0, my = 1.0, dt=0.1;
    while (iter++ < 1070)
    {
        
        if (!kalman.isInit())
        {
            Eigen::VectorXd x_in(4, 1);
            x_in << mx, my, 0.0, 0.0;
            kalman.init(x_in);

            Eigen::MatrixXd P_in(4, 4);
            P_in << 1.0, 0.0, 0.0, 0.0,
                    0.0, 1.0, 0.0, 0.0,
                    0.0, 0.0, 100.0, 0.0,
                    0.0, 0.0, 0.0, 100.0;
            kalman.setP(P_in);


            Eigen::MatrixXd Q_in(4, 4);
            Q_in << 1.0, 0.0, 0.0, 0.0,
                    0.0, 1.0, 0.0, 0.0,
                    0.0, 0.0, 1.0, 0.0,
                    0.0, 0.0, 0.0, 1.0;

            kalman.setQ(Q_in);

            Eigen::MatrixXd H_in(2, 4);
            H_in << 1.0, 0.0, 0.0, 0.0,
                    0.0, 1.0, 0.0, 0.0;

            kalman.setH(H_in);

            Eigen::MatrixXd R_in(2, 2);
            R_in << 0.0225, 0.0,
                    0.0, 0.0225;
            
            kalman.setR(R_in);
            continue;
        }
        
        Eigen::MatrixXd F_in(4, 4);
        F_in << 1.0, 0.0, dt, 0.0,
                0.0, 1.0, 0.0, dt,
                0.0, 0.0, 1.0, 0.0,
                0.0, 0.0, 0.0, 1.0;

        kalman.setF(F_in);
        kalman.predict();

        Eigen::VectorXd z(2, 1);
        z << mx, my;
        kalman.measurement_update(z);


        Eigen::VectorXd x_out = kalman.getX();
        std::cout << "x: " << x_out(0) << ", y:" << x_out(1) << std::endl;
        // std::cout << "vx: " << x_out(2) << ", vy:" << x_out(3) << std::endl;

        std::cout << "mx: " << mx << ", my:" << my << std::endl;
        

        // dt = 0.1;
        mx += 0.3;
        my += 0.4;

        if(iter == 998) mx = 0, my = 0;
        if(iter == 999) mx = 300, my = 399;
    }
    



    return 0;
}