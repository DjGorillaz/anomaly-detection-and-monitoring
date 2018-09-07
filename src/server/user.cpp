#include "user.h"

namespace AnomalyDetection
{
    User::User(const QString& name, const QString& ip_, const quint16& port_, const bool& online_):
        username{name},
        ip{ip_},
        port{port_},
        online{online_},
        cfg{std::make_unique<Config>()},
        offlineTimer{std::make_unique<QTimer>()}
    {
        offlineTimer->setInterval(60000);
        offlineTimer->start();
        QObject::connect(offlineTimer.get(), &QTimer::timeout, [this](){
            setStatus(State::OFFLINE);
        });

        onesided = {0, 0, 0, 0, 0, 0, 0};
        weights = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

        N = 0;
        d0 = 22.2567;
        k = 0.206460;
    }

    User::User(const QString& name, const QString& ip_, const quint16& port_, const bool& online_,
               const double& d0_, const int& N_, const double& k_,
               const QVector<int>& onesided_,
               const QMap<QString, QPair<QVector<double>, QVector <double>>>& features_,
               QVector<float>& weights_):
        User{name, ip_, port_, online_}
    {
        onesided = onesided_;
        weights = weights_;
        features = features_;
        d0 = d0_;
        N = N_;
        k = k_;
    }

    void User::setStatus(State st)
    {
        if (st == State::ONLINE)
        {
            offlineTimer->start();
            online = true;
            emit changedStatus(State::ONLINE, ip);
        }
        else
        {
            offlineTimer->stop();
            online = false;
            emit changedStatus(State::OFFLINE, ip);
        }
    }

    void User::setFeatures(const double &up, const double &upConn, const double &dwn, const double &dwnConn, bool sniffer)
    {
        //Current time and date
        QString currDate = QDate::currentDate().toString("dd.MM.yyyy");;
        QTime currTime = QTime::currentTime();
        double time = currTime.hour()*60 + currTime.minute();

        auto res = features.find(currDate);
        //if current date exists in QMap
        if(res != features.end())
        {
            auto& vector = res.value().first;
            double& firstAccess = vector[0];
            //set first and last access
            if (firstAccess == 0)
            {
                firstAccess = time;
                vector[1] = firstAccess; //lastAccess
                vector[2] = 0; //duration
            }
            else
            {
                vector[1] = time; //lastAccess
                ++vector[2]; //duration
            }

            //set sniffer data
            if (sniffer)
            {
                vector[3] = up;
                vector[4] = upConn;
                vector[5] = dwn;
                vector[6] = dwnConn;
            }

            emit dataChanged(currDate);
        }
        else //if current date doesn't exists
        {
            if (sniffer)
                features.insert(currDate, {{time,time,0,up,upConn,dwn,dwnConn}, {0,0,0,0,0,0,0,0,0}});
            else
                features.insert(currDate, {{time,time,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0}});

            emit newData(currDate);
        }
    }

    void User::setFeatures(bool sniffer)
    {
        setFeatures(0, 0, 0, 0, sniffer);
    }

    QVector<double> User::getScore(const QString& date)
    {
        //Check user's input
        if ((N > features.size()) || (N <= 0))
        {
            auto nan = std::numeric_limits<double>::quiet_NaN();
            return QVector<double>{nan,nan,nan,nan,nan,nan,nan,nan,nan};
        }

        Eigen::ArrayXXd sample(N,7);

        auto iter = features.begin();
        auto const end = features.begin()+N;

        //learning sample
        for(int i = 0; iter != end; ++iter)
        {
            for (int j = 0; j < 7; ++j)
                sample(i,j) = iter.value().first.at(j);
            ++i;
        }

        auto& currFeatures = features[date];

        //input vector
        Eigen::ArrayXd input(7);
        for(int i = 0; i < 7; ++i)
        {
            input(i) = currFeatures.first.at(i);
        }

        Eigen::ArrayXd mean = sample.colwise().mean();
        Eigen::VectorXd centeredInput = input - mean;

        for(int i = 0; i < 7; ++i)
        {
            //one-sided deviations
            if (centeredInput(i)*onesided.at(i) > 0)
                centeredInput(i) = 0;
            //set weights
            else
                centeredInput(i) *= weights.at(i);
        }

        //Calculate distance and score
        Eigen::MatrixXd centeredSample = sample.rowwise() - mean.transpose();
        Eigen::MatrixXd cov = (centeredSample.adjoint() * centeredSample) / double(sample.rows() - 1);

        auto distance = sqrt(centeredInput.transpose() * cov.inverse() * centeredInput);
        double score = 100/(1+exp(k*(d0-distance)));

        currFeatures.second[7] = distance;
        currFeatures.second[8] = round(score*100)/100;

        //Calculate contributions
        Eigen::ArrayXd standardized = static_cast<Eigen::ArrayXd>(centeredInput)/sqrt(static_cast<Eigen::ArrayXd>(cov.diagonal()));

        double sum = 0;
        for(int i = 0; i < 7; ++i)
        {
            double stand = abs(standardized(i));
            currFeatures.second[i] = stand;
            sum += stand;
        }

        for(int i = 0; i < 7; ++i)
            currFeatures.second[i] = round(currFeatures.second[i]/sum*10000)/100;

        return currFeatures.second;
    }
}
