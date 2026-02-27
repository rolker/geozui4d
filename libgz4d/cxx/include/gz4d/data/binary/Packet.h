#ifndef GZ4D_DATA_BINARY_H
#define GZ4D_DATA_BINARY_H

#include<gz4d/time.h>

#pragma pack(1)

namespace gz4d
{
    namespace binary
    {
        struct String
        {
            typedef char value_type;
            uint16_t size;
            char string[];

            String()
            {
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return size;}
        };

        struct CompressedValues8
        {
            typedef uint8_t value_type;
            float scale;
            float offset;
            uint16_t count;
            uint8_t values[];

            CompressedValues8()
            {
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return count;}
        };

        struct Variance3D
        {
            float x;
            float y;
            float z;

            Variance3D()
            {
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct FloatArray
        {
            typedef float value_type;
            uint16_t size;
            float values[];

            FloatArray()
            {
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return size;}
        };

        struct Degrees2D_f
        {
            float x;
            float y;

            Degrees2D_f()
            {
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct BeamGeometry
        {
            /// Across track width is x and along track is y
            Degrees2D_f width;
            /// Positive x is starboard and positive y is towards the bow
            Degrees2D_f direction;

            BeamGeometry()
            {
            }
            bool IsValid() const
            {
                return width.IsValid() && direction.IsValid();
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct Pulse
        {
            /// Pulse length in seconds
            float length;
            /// Pulse frequency in Hz
            float frequency;

            Pulse()
            {
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct RelativePosition3D
        {
            float x;
            float y;
            float z;

            RelativePosition3D()
            {
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct Float3D
        {
            float x;
            float y;
            float z;

            Float3D()
            {
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct CurrentVector
        {
            float depth;
            float error;
            Float3D vector;

            CurrentVector()
            {
            }
            bool IsValid() const
            {
                return vector.IsValid();
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct FourCC
        {
            union
            {
                uint32_t value;
                char string[4];
            };


            FourCC()
            {
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct Fraction
        {
            uint32_t numerator;
            uint32_t denominator;

            Fraction()
            {
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct Packet
        {
            char header[4];
            union
            {
                struct
                {
                    uint8_t major;
                    uint8_t minor;

                    bool IsValid() const
                    {
                        return true;
                    }
                    uint16_t getArrayCount() const {return 0;}
                } version;
                uint16_t fullVersion;
            };

            uint64_t size;
            gz4d::Time timestamp;
            union
            {
                uint32_t fullType;
                struct
                {
                    uint8_t a;
                    uint8_t b;
                    uint8_t c;
                    uint8_t d;

                    bool IsValid() const
                    {
                        return true;
                    }
                    uint16_t getArrayCount() const {return 0;}
                } subType;
            };


            Packet(uint64_t size=26) :size(size),timestamp(gz4d::time::Now()),fullType(0)
            {
                header[0]='g';
                header[1]='z';
                header[2]='4';
                header[3]='d';
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        namespace Metadata
        {
            struct Packet :binary::Packet
            {

                Packet(uint64_t size=sizeof(binary::Packet)+0) :binary::Packet(size)
                {
                    subType.a = 1;
                }
                bool IsValid() const
                {
                    return binary::Packet::IsValid() && subType.a == 1;
                }
                uint16_t getArrayCount() const {return 0;}
            };

            struct Text :Packet
            {
                typedef char value_type;
                uint16_t size;
                char string[];

                Text(uint64_t size=sizeof(Packet)+3) :Packet(size)
                {
                    subType.b = 1;
                }
                bool IsValid() const
                {
                    return Packet::IsValid() && subType.b == 1;
                }
                uint16_t getArrayCount() const {return size;}
            };

        }

        namespace Navigation
        {
            struct Packet :binary::Packet
            {
                uint16_t navChannel;

                Packet(uint64_t size=sizeof(binary::Packet)+2) :binary::Packet(size),navChannel(0)
                {
                    subType.a = 2;
                }
                bool IsValid() const
                {
                    return binary::Packet::IsValid() && subType.a == 2;
                }
                uint16_t getArrayCount() const {return 0;}
            };

            namespace Position
            {
                struct Packet :Navigation::Packet
                {

                    Packet(uint64_t size=sizeof(Navigation::Packet)+0) :Navigation::Packet(size)
                    {
                        subType.b = 1;
                    }
                    bool IsValid() const
                    {
                        return Navigation::Packet::IsValid() && subType.b == 1;
                    }
                    uint16_t getArrayCount() const {return 0;}
                };

                struct Geographic :Packet
                {
                    double latitude;
                    double longitude;

                    Geographic(uint64_t size=sizeof(Packet)+16) :Packet(size)
                    {
                        subType.c = 1;
                    }
                    bool IsValid() const
                    {
                        return Packet::IsValid() && subType.c == 1;
                    }
                    uint16_t getArrayCount() const {return 0;}
                };

                struct UTM :Packet
                {
                    double easting;
                    double northing;
                    uint8_t zone;
                    uint8_t latitudeZone;

                    UTM(uint64_t size=sizeof(Packet)+18) :Packet(size),zone(0),latitudeZone(0)
                    {
                        subType.c = 2;
                    }
                    bool IsValid() const
                    {
                        return Packet::IsValid() && subType.c == 2;
                    }
                    uint16_t getArrayCount() const {return 0;}
                };

            }

            namespace Position3D
            {
                struct Packet :Navigation::Packet
                {

                    Packet(uint64_t size=sizeof(Navigation::Packet)+0) :Navigation::Packet(size)
                    {
                        subType.b = 2;
                    }
                    bool IsValid() const
                    {
                        return Navigation::Packet::IsValid() && subType.b == 2;
                    }
                    uint16_t getArrayCount() const {return 0;}
                };

                struct Geographic :Packet
                {
                    double latitude;
                    double longitude;
                    float z;

                    Geographic(uint64_t size=sizeof(Packet)+20) :Packet(size)
                    {
                        subType.c = 1;
                    }
                    bool IsValid() const
                    {
                        return Packet::IsValid() && subType.c == 1;
                    }
                    uint16_t getArrayCount() const {return 0;}
                };

            }

            struct Attitude :Packet
            {
                float heading;
                /// Positive pitch is nose down
                float pitch;
                /// Positive roll is towards starboard (right)
                float roll;

                Attitude(uint64_t size=sizeof(Packet)+12) :Packet(size)
                {
                    subType.b = 3;
                }
                bool IsValid() const
                {
                    return Packet::IsValid() && subType.b == 3;
                }
                uint16_t getArrayCount() const {return 0;}
            };

            struct Heading :Packet
            {
                float heading;

                Heading(uint64_t size=sizeof(Packet)+4) :Packet(size)
                {
                    subType.b = 4;
                }
                bool IsValid() const
                {
                    return Packet::IsValid() && subType.b == 4;
                }
                uint16_t getArrayCount() const {return 0;}
            };

            namespace Position6DOF
            {
                struct Packet :Navigation::Packet
                {
                    uint16_t positionType;

                    Packet(uint64_t size=sizeof(Navigation::Packet)+2) :Navigation::Packet(size)
                    {
                        subType.b = 5;
                    }
                    bool IsValid() const
                    {
                        return Navigation::Packet::IsValid() && subType.b == 5;
                    }
                    uint16_t getArrayCount() const {return 0;}
                };

                struct Geographic :Packet
                {
                    double latitude;
                    double longitude;
                    float z;
                    float heading;
                    /// Positive pitch is nose down
                    float pitch;
                    /// Positive roll is towards starboard (right)
                    float roll;

                    Geographic(uint64_t size=sizeof(Packet)+32) :Packet(size)
                    {
                        subType.c = 1;
                    }
                    bool IsValid() const
                    {
                        return Packet::IsValid() && subType.c == 1;
                    }
                    uint16_t getArrayCount() const {return 0;}
                };

            }

        }

        namespace Sonar
        {
            struct Packet :binary::Packet
            {
                uint16_t sonarSource;

                Packet(uint64_t size=sizeof(binary::Packet)+2) :binary::Packet(size),sonarSource(0)
                {
                    subType.a = 3;
                }
                bool IsValid() const
                {
                    return binary::Packet::IsValid() && subType.a == 3;
                }
                uint16_t getArrayCount() const {return 0;}
            };

            namespace Channel
            {
                struct Packet :Sonar::Packet
                {
                    uint32_t channelID;

                    Packet(uint64_t size=sizeof(Sonar::Packet)+4) :Sonar::Packet(size),channelID(0)
                    {
                        subType.b = 1;
                    }
                    bool IsValid() const
                    {
                        return Sonar::Packet::IsValid() && subType.b == 1;
                    }
                    uint16_t getArrayCount() const {return 0;}
                };

                struct BottomDetection :Packet
                {
                    typedef FloatArray::value_type value_type;
                    double latitude;
                    double longitude;
                    float heading;
                    float x;
                    float y;
                    float z;
                    Variance3D variance;
                    FloatArray samples;

                    BottomDetection(uint64_t size=sizeof(Packet)+50) :Packet(size)
                    {
                        subType.c = 1;
                    }
                    bool IsValid() const
                    {
                        return Packet::IsValid() && subType.c == 1 && variance.IsValid();
                    }
                    uint16_t getArrayCount() const {return samples.getArrayCount();}
                };

                struct TargetDetection :Packet
                {
                    typedef FloatArray::value_type value_type;
                    double latitude;
                    double longitude;
                    float heading;
                    float x;
                    float y;
                    float z;
                    Variance3D variance;
                    FloatArray samples;

                    TargetDetection(uint64_t size=sizeof(Packet)+50) :Packet(size)
                    {
                        subType.c = 2;
                    }
                    bool IsValid() const
                    {
                        return Packet::IsValid() && subType.c == 2 && variance.IsValid();
                    }
                    uint16_t getArrayCount() const {return samples.getArrayCount();}
                };

                struct Samples :Packet
                {
                    typedef FloatArray::value_type value_type;
                    BeamGeometry geometry;
                    Pulse pulse;
                    float sampleInterval;
                    float offset;
                    FloatArray samples;

                    Samples(uint64_t size=sizeof(Packet)+38) :Packet(size),offset(0.0)
                    {
                        subType.c = 4;
                    }
                    bool IsValid() const
                    {
                        return Packet::IsValid() && subType.c == 4 && geometry.IsValid() && pulse.IsValid();
                    }
                    uint16_t getArrayCount() const {return samples.getArrayCount();}
                };

                struct SamplesCompressed :Packet
                {
                    typedef CompressedValues8::value_type value_type;
                    BeamGeometry geometry;
                    Pulse pulse;
                    float sampleInterval;
                    float offset;
                    CompressedValues8 samples;

                    SamplesCompressed(uint64_t size=sizeof(Packet)+43) :Packet(size),offset(0.0)
                    {
                        subType.c = 5;
                    }
                    bool IsValid() const
                    {
                        return Packet::IsValid() && subType.c == 5 && geometry.IsValid() && pulse.IsValid();
                    }
                    uint16_t getArrayCount() const {return samples.getArrayCount();}
                };

            }

            struct SoundSpeed :Packet
            {
                float soundSpeed;

                SoundSpeed(uint64_t size=sizeof(Packet)+4) :Packet(size)
                {
                    subType.b = 2;
                }
                bool IsValid() const
                {
                    return Packet::IsValid() && subType.b == 2;
                }
                uint16_t getArrayCount() const {return 0;}
            };

            struct SoundSpeedProfile :Packet
            {
                typedef float value_type;
                double latitude;
                double longitude;
                float startDepth;
                float interval;
                uint16_t count;
                float speeds[];

                SoundSpeedProfile(uint64_t size=sizeof(Packet)+30) :Packet(size)
                {
                    subType.b = 3;
                }
                bool IsValid() const
                {
                    return Packet::IsValid() && subType.b == 3;
                }
                uint16_t getArrayCount() const {return count;}
            };

            struct CurrentProfile :Packet
            {
                typedef CurrentVector value_type;
                uint16_t count;
                CurrentVector vectors[];

                CurrentProfile(uint64_t size=sizeof(Packet)+22) :Packet(size)
                {
                    subType.b = 4;
                }
                bool IsValid() const
                {
                    return Packet::IsValid() && subType.b == 4;
                }
                uint16_t getArrayCount() const {return count;}
            };

        }

        namespace Device
        {
            struct Packet :binary::Packet
            {

                Packet(uint64_t size=sizeof(binary::Packet)+0) :binary::Packet(size)
                {
                    subType.a = 4;
                }
                bool IsValid() const
                {
                    return binary::Packet::IsValid() && subType.a == 4;
                }
                uint16_t getArrayCount() const {return 0;}
            };

            struct Polhemus :Packet
            {
                float x;
                float y;
                float z;
                float q1;
                float q2;
                float q3;
                float q4;

                Polhemus(uint64_t size=sizeof(Packet)+28) :Packet(size)
                {
                    subType.b = 1;
                }
                bool IsValid() const
                {
                    return Packet::IsValid() && subType.b == 1;
                }
                uint16_t getArrayCount() const {return 0;}
            };

        }

        namespace Image
        {
            struct Packet :binary::Packet
            {

                Packet(uint64_t size=sizeof(binary::Packet)+0) :binary::Packet(size)
                {
                    subType.a = 5;
                }
                bool IsValid() const
                {
                    return binary::Packet::IsValid() && subType.a == 5;
                }
                uint16_t getArrayCount() const {return 0;}
            };

            struct Frame :Packet
            {
                typedef uint8_t value_type;
                uint16_t width;
                uint16_t height;
                FourCC fourcc;
                uint32_t frameNumber;
                uint32_t dataSize;
                uint8_t data[];

                Frame(uint64_t size=sizeof(Packet)+17) :Packet(size)
                {
                    subType.b = 1;
                }
                bool IsValid() const
                {
                    return Packet::IsValid() && subType.b == 1 && fourcc.IsValid();
                }
                uint32_t getArrayCount() const {return dataSize;}
            };

            namespace Stream
            {
                struct Packet :Image::Packet
                {
                    uint16_t streamID;

                    Packet(uint64_t size=sizeof(Image::Packet)+2) :Image::Packet(size)
                    {
                        subType.b = 2;
                    }
                    bool IsValid() const
                    {
                        return Image::Packet::IsValid() && subType.b == 2;
                    }
                    uint16_t getArrayCount() const {return 0;}
                };

                struct Header :Packet
                {
                    uint16_t width;
                    uint16_t height;
                    Fraction framePeriod;
                    FourCC fourcc;

                    Header(uint64_t size=sizeof(Packet)+16) :Packet(size)
                    {
                        subType.c = 1;
                    }
                    bool IsValid() const
                    {
                        return Packet::IsValid() && subType.c == 1 && framePeriod.IsValid() && fourcc.IsValid();
                    }
                    uint16_t getArrayCount() const {return 0;}
                };

                struct Frame :Packet
                {
                    typedef uint8_t value_type;
                    uint32_t frameNumber;
                    uint32_t dataSize;
                    uint8_t data[];

                    Frame(uint64_t size=sizeof(Packet)+9) :Packet(size)
                    {
                        subType.c = 2;
                    }
                    bool IsValid() const
                    {
                        return Packet::IsValid() && subType.c == 2;
                    }
                    uint32_t getArrayCount() const {return dataSize;}
                };

            }

        }

    }
}

#pragma pack()

#endif
