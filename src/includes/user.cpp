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
    d0 = 30;
    k = 0.15;

    features.insert("11.04.2018", {465, 1025, 500, 944, 5294, 9401, 8878});
    features.insert("12.04.2018", {470, 1010, 489, 660, 3594, 7818, 6921});
    features.insert("13.04.2018", {455, 1050, 450, 858, 3477, 4781, 5253});
    features.insert("14.04.2018", {440, 1021, 400, 1000, 4500, 6000, 6341});
    features.insert("15.04.2018", {492, 1031, 520, 300, 6700, 7104, 7234});
    features.insert("16.04.2018", {453, 1030, 467, 1500, 4100, 5400, 6500});
    features.insert("17.04.2018", {463, 1021, 481, 700, 5123, 6123, 4012});
    features.insert("18.04.2018", {423, 1015, 484, 650, 4194, 7853, 7800});
    features.insert("19.04.2018", {445, 1040, 490, 800, 6100, 5323, 5453});
    features.insert("20.04.2018", {459, 1034, 460, 950, 7600, 8321, 6452});

    //Поздно зашёл
    features.insert("27.04.2018", {451, 1420, 530, 944, 5294, 7301, 6878});
    //Рано пришёл, долго работал
    features.insert("28.04.2018", {200, 1000, 800, 944, 5294, 7301, 6878});
    //Очень много скачал/раздал
    features.insert("29.04.2018", {460, 1008, 473, 1142701, 1601887, 1451901, 1716163});
    //Пришёл в середине дня, рано ушёл
    features.insert("30.04.2018", {800, 900, 100, 944, 5294, 7301, 6878});
    //Пришёл нормально, рано ушёл
    features.insert("31.04.2018", {450, 750, 350, 944, 5294, 7301, 6878});
    //Много отдал
    features.insert("32.04.2018", {453, 1030, 467, 3200, 12000, 5400, 6500});
    //Отдал в 1,5 раза больше и скачал
    features.insert("33.04.2018", {492, 1031, 520, 2000, 8000, 11000, 8500});
    //Скачал много
    features.insert("34.04.2018", {460, 1015, 460, 919, 9203, 55864, 41408});
}

User::User(const QString& name, const QString& ip_, const quint16& port_, const bool& online_,
           const double& d0_, const uint& N_, const double& k_,
           const QVector<int>& onesided_, const QMap<QString, QVector<double>>& features_, QVector<float>& weights_):
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
        auto& vector = res.value();
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
            features.insert(currDate, {time,time,0,up,upConn,dwn,dwnConn});
        else
            features.insert(currDate, {time,time,0,0,0,0,0});

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
            sample(i,j) = iter.value().at(j);
        ++i;
    }

    //input vector
    Eigen::ArrayXd input(7);
    for(int i = 0; i < 7; ++i)
    {
        input(i) = features[date].at(i);
    }

    Eigen::ArrayXd mean = sample.colwise().mean();
    Eigen::MatrixXd centered = sample.rowwise() - mean.transpose();
    Eigen::MatrixXd cov = (centered.adjoint() * centered) / double(sample.rows() - 1);
    Eigen::VectorXd delta = input - mean;

    for(int i = 0; i < 7; ++i)
    {
        //one-sided deviations
        if (delta(i)*onesided.at(i) > 0)
            delta(i) = 0;
        //set weights
        else
            delta(i) *= weights.at(i);
    }

    //Calculate distance
    auto distance = sqrt(delta.transpose() * cov.inverse() * delta);

    //TODO del
    qDebug() << distance;
    double score = 100/(1+exp(k*(d0-distance)));

    //get ci
    QVector<double> ci;
    ci.reserve(9);
    ci.append(round(score*100)/100);

    Eigen::ArrayXd std_dev = ((sample.rowwise() - mean.transpose()).square().colwise().sum()/(N-1)).sqrt();
    Eigen::ArrayXd standardized = static_cast<Eigen::ArrayXd>(delta)/std_dev;

    double summ = 0;
    for(int i = 0; i < 7; ++i)
    {
        double stand = abs(standardized(i));
        ci.append(stand);
        summ += stand;
    }
    ci.append(summ);

    return ci;
}

