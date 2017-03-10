#include "fftw3.h"

#include <QTime>
#include <QDebug>

typedef long long bigint;

struct Fftw_runner {
    Fftw_runner() {

    }

    ~Fftw_runner() {
        fftw_free(data_in);
        fftw_free(data_out);
    }
    void init(bigint M_in,bigint N_in,QString task_in) {
        M=M_in;
        N=N_in;
        MN=M*N;
        task=task_in;

        data_in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * MN);
        data_out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * MN);

        int rank = 1;
        int n[] = { (int)N };
        int howmany = M;
        int* inembed = n;
        int istride = M;
        int idist = 1;
        int* onembed = n;
        int ostride = M;
        int odist = 1;
        unsigned flags = FFTW_ESTIMATE;
        if (task=="fftw") {
            qDebug() << "Creating plan...";
            p_fft = fftw_plan_many_dft(rank,n,howmany,data_in,inembed,istride,idist,data_out,onembed,ostride,odist,FFTW_FORWARD,flags);
        }

    }
    void apply() {
        if (task=="fftw") {
            for (bigint i=0; i<MN; i++) {
                data_in[i][0]=0;
                data_in[i][1]=0;
            }
            //set input data
            //fft
            fftw_execute(p_fft);
            //multiply by kernel
        }
        else if (task=="flops") {
            double sum=0;
            for (bigint n=0; n<N; n++) {
                for (bigint m=0; m<M; m++) {
                    sum+=m*n;
                }
            }
            printf("Print the result to force computation: %g\n",sum);
        }
    }

    bigint M;
    bigint N,MN;
    QString task;
    fftw_complex* data_in=0;
    fftw_complex* data_out=0;
    fftw_plan p_fft;
};

class CLParams {
public:
    CLParams(int argc, char* argv[]);
    QMap<QString, QVariant> named_parameters;
    QList<QString> unnamed_parameters;
    bool success;
    QString error_message;
};

int main(int argc,char *argv[]) {
    CLParams params(argc,argv);

    QString task=params.named_parameters.value("task","fftw").toString();
    bigint default_M=1;
    if (task=="fftw") default_M=4;
    else if (task=="flops") default_M=100;
    bigint N=params.named_parameters.value("N",4e7).toDouble();
    bigint M=params.named_parameters.value("M",default_M).toDouble();


    Fftw_runner FR;
    FR.init(M,N,task);

    qDebug().noquote() << QString("Running fftw load test for M,N=%1,%2").arg(M).arg(N);
    QTime timer; timer.start();
    FR.apply();
    double rate=M*N/(timer.elapsed()*1.0/1000);
    qDebug().noquote() << QString("Elapsed time (sec) for M,N=%1,%2 (%3 nums per sec):").arg(M).arg(N).arg(rate) << timer.elapsed()*1.0/1000;

    return 0;
}



QVariant clp_string_to_variant(const QString& str);

CLParams::CLParams(int argc, char* argv[])
{
    this->success = true; //let's be optimistic!

    //find the named and unnamed parameters checking for errors along the way
    for (int i = 1; i < argc; i++) {
        QString str = QString(argv[i]);
        if (str.startsWith("--")) {
            int ind2 = str.indexOf("=");
            QString name = str.mid(2, ind2 - 2);
            QString val = "";
            if (ind2 >= 0)
                val = str.mid(ind2 + 1);
            if (name.isEmpty()) {
                this->success = false;
                this->error_message = "Problem with parameter: " + str;
                return;
            }
            QVariant val2 = clp_string_to_variant(val);
            if (this->named_parameters.contains(name)) {
                QVariant tmp = this->named_parameters[name];
                QVariantList list;
                if (tmp.type() == QVariant::List) {
                    list = tmp.toList();
                }
                else {
                    list.append(tmp);
                }
                if (val2.type() == QVariant::List)
                    list.append(val2.toList());
                else
                    list.append(val2);
                this->named_parameters[name] = list;
            }
            else {
                this->named_parameters[name] = val2;
            }
        }
        else {
            this->unnamed_parameters << str;
        }
    }
}

bool clp_is_long(const QString& str)
{
    bool ok;
    str.toLongLong(&ok);
    return ok;
}

bool clp_is_int(const QString& str)
{
    bool ok;
    str.toInt(&ok);
    return ok;
}

bool clp_is_float(const QString& str)
{
    bool ok;
    str.toFloat(&ok);
    return ok;
}

QVariant clp_string_to_variant(const QString& str)
{
    if (clp_is_long(str))
        return str.toLongLong();
    if (clp_is_int(str))
        return str.toInt();
    if (clp_is_float(str))
        return str.toFloat();
    if ((str.startsWith("[")) && (str.endsWith("]"))) {
        QString str2 = str.mid(1, str.count() - 2);
        QStringList list = str2.split("][");
        if (list.count() == 1) {
            return list[0];
        }
        else {
            QVariantList ret;
            foreach (QString tmp, list)
                ret.append(tmp);
            return ret;
        }
    }
    return str;
}
