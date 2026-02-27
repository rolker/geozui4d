#ifndef GZ4D_GRID_H
#define GZ4D_GRID_H

#include <gz4d/gz4d.h>
#include <gz4d/glo/DisplayList.h>
#include <gz4d/Color.h>
#include <gz4d/Point.h>
#include <gz4d/glo/Vertex.h>
#include <gz4d/glo/Color.h>
#include <vector>


namespace gz4d
{
    template<typename PointType>
    class Grid: public glo::DisplayList
    {
        struct Dimension
        {
            PointType increment;
            int count;
            Dimension():count(0){}
            Dimension(PointType i, int c):increment(i),count(c){}
            Dimension(Dimension const &d):increment(d.increment),count(d.count){}
        };

        std::vector<Dimension> dimensions;
        PointType startPoint;
        Color color;
        float lineWidth;

        void DrawDimension(PointType point, int level)
        {
            if(level == 0)
                glBegin(GL_LINE_STRIP);

            for(int i = 0; i <= dimensions[level].count; ++i, point += dimensions[level].increment)
                if(level == 0)
                    glo::Vertex(gz4d::Point<float>(point));
                else
                    DrawDimension(point, level-1);
            if(level == 0)
                glEnd();
        }

        public:
            Grid():lineWidth(1.0){}
            void AddDimension(PointType i, int c){dimensions.push_back(Dimension(i,c));Rebuild();}
            void SetLineWidth(float w){lineWidth = w;Rebuild();}
            void SetColor(Color c){color = c;Rebuild();}
            void SetStartPoint(PointType s){startPoint = s;Rebuild();}

            void Draw()
            {
                if(dimensions.size())
                {

                    if(!Call())
                    {
                        glPushAttrib(GL_LINE_BIT|GL_LIGHTING_BIT);
                        glLineWidth(lineWidth);
                        glDisable(GL_LIGHTING);
                        glo::Color(color);
                        DrawDimension(startPoint,dimensions.size()-1);
                        glPopAttrib();
                        End();
                    }
                }
            }
    };
}
#endif
