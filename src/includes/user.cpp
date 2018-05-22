#include "user.h"

User::User(const QString& name, const QString& ip_, const quint16& port_, const bool& online_):
    username{name},
    ip{ip_},
    port{port_},
    online{online_},
    offlineTimer{std::make_unique<QTimer>()},
    cfg{std::make_unique<Config>()}
{
    offlineTimer->setInterval(60000);
    offlineTimer->start();
    QObject::connect(offlineTimer.get(), &QTimer::timeout, [this](){
        setStatus(State::OFFLINE);
    });

    onesided = {0, 0, 0, 0, 0, 0, 0};
    weights = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

    //TODO
    N = 10;
    d0 = 22.2567;
    k = 0.206460;

    features.insert("01.04.2018", {{460, 1015, 460, 230321, 1432, 865328, 9851}, {0,0,0,0,0,0,0,0,0}});
    features.insert("02.04.2018", {{465, 1025, 500, 84123, 875, 1021358, 8878}, {0,0,0,0,0,0,0,0,0}});
    features.insert("03.04.2018", {{470, 1010, 489, 142142, 1325, 798356, 6921}, {0,0,0,0,0,0,0,0,0}});
    features.insert("04.04.2018", {{455, 1050, 450, 120213, 1320, 869132, 6253}, {0,0,0,0,0,0,0,0,0}});
    features.insert("05.04.2018", {{440, 1021, 400, 92134, 761, 896531, 6341}, {0,0,0,0,0,0,0,0,0}});
    features.insert("06.04.2018", {{453, 1030, 467, 171341, 1236, 810486, 6500}, {0,0,0,0,0,0,0,0,0}});
    features.insert("07.04.2018", {{463, 1021, 481, 124546, 1068, 612543, 5012}, {0,0,0,0,0,0,0,0,0}});
    features.insert("08.04.2018", {{423, 1015, 484, 98231, 1073, 754853, 7800}, {0,0,0,0,0,0,0,0,0}});
    features.insert("09.04.2018", {{445, 1040, 490, 130143, 964, 981325, 7453}, {0,0,0,0,0,0,0,0,0}});
    features.insert("10.04.2018", {{459, 1034, 460, 123415, 1135, 831658, 6452}, {0,0,0,0,0,0,0,0,0}});

    //Test cases
    //Вышел на час позже
    features.insert("21.04.2018", {{451, 1116, 548, 125104, 1143, 853134, 6941}, {0,0,0,0,0,0,0,0,0}});
    //50% больше отдал и скачал
    features.insert("22.04.2018", {{458, 1021, 473, 350123, 2000, 1833310, 11354}, {0,0,0,0,0,0,0,0,0}});
    //Поздно зашёл и дольше работал
    features.insert("23.04.2018", {{451, 1322, 527, 125104, 1143, 991134, 7641}, {0,0,0,0,0,0,0,0,0}});
    //Много скачал
    features.insert("24.04.2018", {{460, 1008, 473, 165315, 1241, 2154312, 10234}, {0,0,0,0,0,0,0,0,0}});
    //Normal (8-й)
    features.insert("25.04.2018", {{423, 1015, 484, 98231, 1073, 754853, 7800}, {0,0,0,0,0,0,0,0,0}});
    //Пришёл в середине дня, рано ушёл
    features.insert("26.04.2018", {{800, 828, 28, 34194, 841, 941244, 6878}, {0,0,0,0,0,0,0,0,0}});
    //Много отдал
    features.insert("27.04.2018", {{453, 1030, 467, 563128, 1234, 965423, 7231}, {0,0,0,0,0,0,0,0,0}});
    //Взломали?
    features.insert("28.04.2018", {{341, 1035, 489, 89421, 942, 942134, 7542}, {0,0,0,0,0,0,0,0,0}});
    //Просканировал подсеть
    features.insert("29.04.2018", {{462, 1047, 465, 89421, 3651, 942134, 7142}, {0,0,0,0,0,0,0,0,0}});

}

User::User(const QString& name, const QString& ip_, const quint16& port_, const bool& online_,
           const double& d0_, const uint& N_, const double& k_,
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
    currFeatures.second[8] = score;

    //Calculate contributions
    Eigen::ArrayXd standardized = static_cast<Eigen::ArrayXd>(centeredInput)/sqrt(static_cast<Eigen::ArrayXd>(cov.diagonal()));

    double summ = 0;
    for(int i = 0; i < 7; ++i)
    {
        double stand = abs(standardized(i));
        currFeatures.second[i] = stand;
        summ += stand;
    }

    for(int i = 0; i < 7; ++i)
        currFeatures.second[i] = round(currFeatures.second[i]/summ*10000)/100;

    return currFeatures.second;
}

