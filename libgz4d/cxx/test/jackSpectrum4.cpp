#include <gz4d/sound/Jack.h>
#include <gz4d/glut/Application.h>

class JackSpectrum: public gz4d::glut::Application<JackSpectrum>
{
    std::shared_ptr<gz4d::jack::Client> client;
    int bufferSize;
    int historyFactor;
    bool showLeds;
    bool showAverage;
    int samplesPerFrame;

    std::vector<gz4d::jack::InputPort::Ptr> ports;
    std::vector<gz4d::ThreadedQueue<gz4d::AudioPacket::Ptr>::Ptr> port_qs;
    std::vector<std::list<gz4d::AudioPacket::Ptr> > audio_packets;
    std::vector<std::list<float> > average_powers;
    std::vector<float> running_averages;
    std::vector<float> running_max_averages;

    public:
        
        JackSpectrum(std::vector<std::string> port_labels, int bufferSize, int historyFactor)
    :client(new gz4d::jack::Client("Spectrum")),bufferSize(bufferSize),historyFactor(historyFactor),showLeds(false),showAverage(true)
        {
            for (std::vector<std::string>::iterator l = port_labels.begin(); l != port_labels.end(); ++l)
            {
                gz4d::jack::InputPort::Ptr p = client->NewInputPort(*l);
                gz4d::ThreadedQueue<gz4d::AudioPacket::Ptr>::Ptr pq = p->NewOutQueue();
                ports.push_back(p);
                port_qs.push_back(pq);
            }
            audio_packets.resize(ports.size());
            average_powers.resize(ports.size());
            running_averages.resize(ports.size());
            running_max_averages.resize(ports.size());
            samplesPerFrame = client->GetBufferSize();
            client->Activate();
        }

        void Idle()
        {
            for(int i = 0; i < ports.size(); ++i)
            {
                while(!port_qs[i]->empty())
                {
                    gz4d::AudioPacket::Ptr ap = port_qs[i]->pop();
                    float avg = 0.0, maxp = 0.0;
                    for(std::vector<float>::iterator s = ap->samples.begin(); s != ap->samples.end(); ++s)
                    {
                        *s *= *s;
                        avg += *s;
                        maxp = std::max(maxp,*s);
                    }
                    avg /= (float)(ap->samples.size());

                    running_averages[i] *= 0.98;
                    running_averages[i] += 0.02*avg;

                    running_max_averages[i] *= 0.9;
                    running_max_averages[i] += 0.1*maxp;

                    average_powers[i].push_back(avg);
                    if(average_powers[i].size() > bufferSize*historyFactor)
                        average_powers[i].pop_front();

                    audio_packets[i].push_back(ap);
                    if(audio_packets[i].size() > bufferSize)
                        audio_packets[i].pop_front();
                    PostRedisplay();
                }
            }
        }

        void Draw()
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDrawBuffer(GL_BACK);
            glPushMatrix();
            float global_average = 0.0;
            float min_max_average = running_max_averages[0];
            for(int i = 0; i < ports.size(); ++i)
            {
                global_average += running_averages[i];
                min_max_average = std::min(running_max_averages[i],min_max_average);
            }
            global_average /= ports.size();

            float gain = 0.01 / global_average;
            gain = std::max(gain,(float)(1.0));
            //std::cerr << "average: " << global_average << "\tgain: " << gain << std::endl;

            float cgain = 20.0;
            if(showLeds)
                cgain *= 0.5;

            float hgain = 10.0;

            std::vector<float> maxPower(ports.size(),0.0);

            for(int i = 0; i < ports.size(); ++i)
            {
                glPushMatrix();
                switch(i)
                {
                    case 1: // right
                        glRotatef(180.0,0.0,0.0,1.0);
                        break;
                        case 2: // forward
                            glRotatef(-90.0,0.0,0.0,1.0);
                            break;
                            case 3: // aft
                                glRotatef(90.0,0.0,0.0,1.0);
                                break;
                    default:
                        break;
                }

                glBegin(GL_LINE_STRIP);
                float j = 0.0;
                for(std::list<float>::iterator p = average_powers[i].begin(); p != average_powers[i].end(); ++p)
                {
                    glColor3f(*p*gain*cgain*1.2,*p*gain*cgain,*p*gain*cgain);
                    glVertex3f(j-average_powers[i].size()/float(historyFactor),(*p)*gain*hgain*bufferSize,-0.1);
                    //glVertex3f(j-average_powers[i].size()/float(historyFactor),-(*p)*gain*hgain*bufferSize,-0.1);
                    j += 1.0/float(historyFactor);
                }
                glEnd();

                glBegin(GL_LINE_STRIP);
                j = 0.0;
                for(std::list<float>::iterator p = average_powers[i].begin(); p != average_powers[i].end(); ++p)
                {
                    glColor3f(*p*gain*cgain*1.2,*p*gain*cgain,*p*gain*cgain);
                    glVertex3f(j-average_powers[i].size()/float(historyFactor),-(*p)*gain*hgain*bufferSize,-0.1);
                    j += 1.0/float(historyFactor);
                }
                glEnd();

                glPopMatrix();
            }

            for(int i = 0; i < ports.size(); ++i)
            {
                glPushMatrix();
                switch(i)
                {
                    case 1: // right
                        glRotatef(180.0,0.0,0.0,1.0);
                        break;
                    case 2: // forward
                        glRotatef(-90.0,0.0,0.0,1.0);
                        break;
                    case 3: // aft
                        glRotatef(90.0,0.0,0.0,1.0);
                        break;
                    default:
                        break;
                }


                if(!showAverage)
                {
                    glBegin(GL_LINES);
                    float j = 0.0;
                    for(std::list<gz4d::AudioPacket::Ptr>::iterator ap = audio_packets[i].begin(); ap != audio_packets[i].end(); ++ap)
                    {
                        for(int k = 0; k < (*ap)->samples.size(); ++k)
                        {
                            float p = (*ap)->samples[k];
                            if(showLeds)
                                glColor3f(p*0.4*gain*cgain,p*1.0*gain*cgain,p*0.4*gain*cgain);
                            else
                                glColor3f(p*1.0*gain*cgain,p*0.4*gain*cgain,p*0.4*gain*cgain);
                            maxPower[i] = std::max(maxPower[i],p);
                            glVertex3f(j+k/float(samplesPerFrame)-bufferSize,p*gain*bufferSize,0.0);
                            glVertex3f(j+k/float(samplesPerFrame)-bufferSize,-p*gain*bufferSize,0.0);
                        }
                        j += 1.0;
                    }
                    glEnd();
                }
                else
                {
                    glBegin(GL_QUAD_STRIP);
                    float j = bufferSize;
                    for(std::list<float>::reverse_iterator p = average_powers[i].rbegin(); p != average_powers[i].rend() && j >=0.0; ++p)
                    {
                        if(showLeds)
                            glColor3f(*p*0.4*gain*cgain,*p*1.0*gain*cgain,*p*0.4*gain*cgain);
                        else
                            glColor3f(*p*1.0*gain*cgain,*p*0.4*gain*cgain,*p*0.4*gain*cgain);
                        maxPower[i] = std::max(maxPower[i],*p);
                        glVertex3f(j-bufferSize,(*p)*gain*bufferSize,0.0);
                        glVertex3f(j-bufferSize,-(*p)*gain*bufferSize,0.0);
                        j -= 1.0;
                    }
                    glEnd();
                }

                glColor3f(0.4,0.0,0.0);
                glBegin(GL_POLYGON);
                    glVertex3f(-bufferSize*0.99,bufferSize*.5*maxPower[i],0.0);
                    glVertex3f(-bufferSize*0.99,-bufferSize*.5*maxPower[i],0.0);
                    glVertex3f(-bufferSize*0.95,-bufferSize*.5*maxPower[i],0.0);
                    glVertex3f(-bufferSize*0.95,bufferSize*.5*maxPower[i],0.0);
                glEnd();

                glColor3f(0.0,0.0,1.0);
                glBegin(GL_LINE_LOOP);
                glVertex3f(-bufferSize*0.99,bufferSize*.5,0.0);
                glVertex3f(-bufferSize*0.99,-bufferSize*.5,0.0);
                glVertex3f(-bufferSize*0.95,-bufferSize*.5,0.0);
                glVertex3f(-bufferSize*0.95,bufferSize*.5,0.0);
                glEnd();

                glBegin(GL_POLYGON);
                glVertex3f(-bufferSize*0.99,bufferSize*.5*running_averages[i],0.0);
                glVertex3f(-bufferSize*0.99,-bufferSize*.5*running_averages[i],0.0);
                glVertex3f(-bufferSize*0.95,-bufferSize*.5*running_averages[i],0.0);
                glVertex3f(-bufferSize*0.95,bufferSize*.5*running_averages[i],0.0);
                glEnd();

                glPopMatrix();
            }
            
            if(showLeds)
            {
                for(int i = 0; i < ports.size(); ++i)
                    min_max_average = std::min(min_max_average,maxPower[i]);
                float lgain = 4.0/min_max_average;

                glPointSize(10);
                for(int i = 0; i < ports.size(); ++i)
                {
                    glPushMatrix();
                    switch(i)
                    {
                        case 1: // right
                            glRotatef(180.0,0.0,0.0,1.0);
                            break;
                            case 2: // forward
                                glRotatef(-90.0,0.0,0.0,1.0);
                                break;
                                case 3: // aft
                                    glRotatef(90.0,0.0,0.0,1.0);
                                    break;
                        default:
                            break;
                    }

                    glBegin(GL_POINTS);
                    for (int k = 0; k < 8; ++k)
                    {
                        if((maxPower[i]-min_max_average)*lgain >= k)
                            glColor3f(1.0,0.0,0.0);
                        else
                            glColor3f(0.2,0.0,0.0);
                        glVertex3f(-bufferSize*(k+1)/9.0,0.0,0.0);
                    }
                    glEnd();
                    glPopMatrix();
                }
            }

            glPopMatrix();
            SwapBuffers();
        }

        void Reshape(int w, int h)
        {
            glViewport(0, 0, w, h);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(-bufferSize, bufferSize,-bufferSize,bufferSize,-1.0,1.0);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
        }

        void Mouse(int button, int state, int x, int y)
        {
            std::cerr << button << std::endl;
        }

        void Motion(int x, int y)
        {
        }

        void Keyboard(unsigned char key, int x, int y)
        {
            if(key == 'q')
            {
                std::cerr << "Exiting..." << std::endl;
                client.reset();
                Sleep(gz4d::seconds(5));
                std::cerr << "Done" << std::endl;
                exit(0);
            }
            if(key == 'l')
                showLeds = !showLeds;
            if(key == 'a')
                showAverage = !showAverage;
        }
};


int main(int argc, char * argv[])
{
    int bufferSize = 25;
    int historyFactor = 20;
    if (argc > 1)
    {
        bufferSize = atoi(argv[1]);
        if(argc > 2)
            historyFactor = atoi(argv[2]);
    }

    std::cerr << "Buffer size: " << bufferSize << std::endl;
    std::cerr << "History factor: " << historyFactor << std::endl;

    std::vector<std::string> ports;
    ports.push_back("left");
    ports.push_back("right");
    ports.push_back("forward");
    ports.push_back("aft");
    JackSpectrum js(ports,bufferSize,historyFactor);
    js("Spectrum");
}
