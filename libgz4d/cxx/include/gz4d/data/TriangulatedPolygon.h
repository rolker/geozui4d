// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_DATA_TRIANGULATEDPOLYGON_H
#define GZ4D_DATA_TRIANGULATEDPOLYGON_H

#include <gz4d/data/Polygon.h>
#include <gz4d/data/Triangle.h>
#include <vector>
#include <set>
#include <map>
#include <gz4d/data/Segment.h>
#include <gz4d/vector/operations.h>
#include <gz4d/Matrix.h>
#include <gz4d/matrix/operations.h>
#include <queue>

namespace gz4d
{
    /// \ingroup container
    template <typename T> struct TriangulatedPolygon
    {
        std::vector<T> vertices;

        struct XYLessThan
        {
            template<typename IT> static bool lt(IT const &a, IT const &b)
            {
                if(a.x == b.x)
                    return a.y < b.y;
                return a.x < b.x;
            }
        };

        struct YXLessThan
        {
            template<typename IT> static bool lt(IT const &a, IT const &b)
            {
                if(a.y == b.y)
                    return a.x < b.x;
                return a.y < b.y;
            }
        };
        
        template<typename LTT> struct Index
        {
            typename T::value_type x,y;

            Index():x(0.0),y(0.0){}
            Index(typename T::value_type x, typename T::value_type y):x(x),y(y){}
            Index(T const &p):x(p[0]),y(p[1]){}
            template<typename OLTT> Index(Index<OLTT> const &i):x(i.x),y(i.y){}

            bool operator<(Index const &o) const
            {
                return LTT::lt(*this,o);
            }
        };

        typedef Index<XYLessThan> XYIndex;
        typedef Index<YXLessThan> YXIndex;

        struct Vertex
        {
            int ring;
            int index;

            /// Constructs invalid vertex
            Vertex():ring(-3),index(0){}
            /// Construct exterior vertex
            Vertex(int i):ring(-1),index(i){}
            /// Construct interior vertex or dummy one with r = -2
            Vertex(int r, int i):ring(r),index(i){}

            bool valid() const {return ring >= -2;}
            bool isInterior() const {return ring >= 0;}
            bool isDummy() const {return ring == -2;}

            bool operator<(Vertex const &o) const
            {
                if(ring == o.ring)
                    return index < o.index;
                return ring < o.ring;
            }

            bool operator!=(Vertex const &o) const
            {
                return ring != o.ring || index != o.index;
            }
            
            bool operator==(Vertex const &o) const
            {
                return !(*this != o);
            }

            bool sameRing(Vertex const &o) const
            {
                return ring == o.ring;
            }
            
            T operator()(Polygon<T> const &p) const
            {
                if(ring == -2)
                    if(index == 0)
                        return p.sw();
                    else
                        return p.nw();
                if(isInterior())
                    return p.getInteriorRings()[ring][index];
                return p.getExteriorRing()[index];
            }

            Vertex next(Polygon<T> const &p) const
            {
                int rsize = 0;
                Vertex ret(ring, index+1);

                if(ring == -2)
                {
                    rsize = 2;
                }
                else if(ring == -1)
                {
                    rsize = p.getExteriorRing().size();
                }
                else
                {
                    rsize = p.getInteriorRings()[ring].size();
                }
                if(ret.index == rsize)
                    ret.index = 0;
                return ret;
            }

            Vertex prev(Polygon<T> const &p) const
            {
                Vertex ret(ring, index-1);
                if(index > 0)
                    return ret;

                int rsize = 0;
                if(ring == -2)
                {
                    rsize = 2;
                }
                else if(ring == -1)
                {
                    rsize = p.getExteriorRing().size();
                }
                else
                {
                    rsize = p.getInteriorRings()[ring].size();
                }
                ret.index = rsize-1;
                return ret;
            }

            std::string str() const
            {
                if(!valid())
                    return "(invalid)";
                std::stringstream ret;
                if(ring == -2)
                    ret << "d:";
                else if (ring == -1)
                    ret << "e:";
                else
                    ret << "i" << ring << ":";
                ret << index;
                return ret.str();
            }
        };

        struct Seg
        {
            Vertex a,b;
            Seg(){}
            Seg(Vertex const &a, Vertex const &b):a(std::min(a,b)),b(std::max(a,b)){assert(a != b);}

            bool valid() const
            {
                return a.valid() && b.valid();
            }

            bool intersects(Seg const &s, Polygon<T> const &p) const
            {
                if(a == s.a || a == s.b || b == s.a || b == s.b)
                    return false;
                return Segment<T>(a(p),b(p)).intersects(Segment<T>(s.a(p),s.b(p)));
            }

            bool fixed(Polygon<T> const &p) const
            {
                return a.next(p) == b || b.next(p) == a;
            }

            bool contains(Vertex const &v) const
            {
                return a == v || b == v;
            }
            
            std::string str() const
            {
                std::stringstream ret;
                ret << "(" << a.str() << ", " << b.str() << ")";
                return ret.str();
            }

            std::string str(Polygon<T> const &p) const
            {
                std::stringstream ret;
                ret << str() << " (" << a(p)-p.sw() << ", " << b(p) - p.sw() << ")";
                return ret.str();
            }
            
            std::string str(Polygon<T> const &p, Vertex const &base) const
            {
                std::stringstream ret;
                ret << str() << " (" << a(p)-base(p) << ", " << b(p) - base(p) << ")";
                return ret.str();
            }
            


            Segment<T> operator()(Polygon<T> const &p) const
            {
                return Segment<T>(a(p),b(p));
            }

            bool operator!=(Seg const &o) const
            {
                return a != o.a || b != o.b;
            }

            bool operator<(Seg const &o) const
            {
                if (a == o.a)
                    return b < o.b;
                return a < o.a;
            }
        };

        struct Tri
        {
            Vertex a,b,c;
            Tri(){}
            Tri(Vertex const &va, Vertex const &vb, Vertex const &vc):a(va),b(vb),c(vc)
            {
                if(c<b)
                    std::swap(Tri::c, Tri::b);
                if(b<a)
                    std::swap(Tri::a, Tri::b);
                if(c<b)
                    std::swap(Tri::c, Tri::b);
                assert(a<b && b < c);
            }

            bool valid() const
            {
                return a.valid() && b.valid() && c.valid();
            }

            Triangle<T> operator()(Polygon<T> const &p) const
            {
                return Triangle<T>(a(p),b(p),c(p));
            }


            bool operator<(Tri const &o) const
            {
                if(a == o.a)
                    if(b == o.b)
                        return c < o.c;
                    else
                        return b < o.b;
                return a < o.a;
            }

            bool operator!=(Tri const &o) const
            {
                return a != o.a || b != o.b || c != o.c;
            }

            bool operator==(Tri const &o) const
            {
                return  !(*this != o);
            }

            bool contains(Seg const &s) const
            {
                return (s.a == a || s.a == b) && (s.b == b || s.b == c);
            }

            bool contains(Vertex const &v) const
            {
                return (v == a || v == b || v == c);
            }


            Seg getOppositeSegment(Vertex const &v) const
            {
                if(v == a)
                    return Seg(b,c);
                if(v == b)
                    return Seg(a,c);
                assert(v == c);
                return Seg(a,b);
            }

            Vertex getOppositeVertex(Seg const &s) const
            {
                if(!s.contains(a))
                    return a;
                if(!s.contains(b))
                    return b;
                assert(!s.contains(c));
                return c;
            }

            Seg shared(Tri const &t) const
            {
                if(!t.contains(a))
                    return Seg(b,c);
                if(!t.contains(b))
                    return Seg(a,c);
                assert(!t.contains(c));
                return Seg(a,b);
            }

            std::string str() const
            {
                std::stringstream ret;
                ret << "<" << a.str() << "," << b.str() << "," << c.str() << ">";
                return ret.str();
            }

            std::string str(Polygon<T> const &p) const
            {
                std::stringstream ret;
                ret << str() << " <" << a(p)-p.sw() << b(p)-p.sw() << c(p)-p.sw() << ">";
                return ret.str();
            }
            
            std::string str(Polygon<T> const &p, Vertex const &base) const
            {
                std::stringstream ret;
                ret << str() << " <" << a(p)-base(p) << b(p)-base(p) << c(p)-base(p) << ">";
                return ret.str();
            }
            
        };

        struct TriList
        {
            std::map<Vertex,std::set<Tri> > vertMap;
            std::set<Tri> triangles;
            void insert(Tri const &t)
            {
                //std::cerr << "\t\tinserting " << t.str() << std::endl;
                if(triangles.count(t))
                {
                    std::cerr << t.str() << " already there!" << std::endl;
                }
                assert(triangles.count(t) == 0);
                triangles.insert(t);
                vertMap[t.a].insert(t);
                vertMap[t.b].insert(t);
                vertMap[t.c].insert(t);
            }
            void erase(Tri const &t)
            {
                assert(triangles.count(t));
                //std::cerr << "\t\terasing " << t.str() << std::endl;
                triangles.erase(t);
                vertMap[t.a].erase(t);
                vertMap[t.b].erase(t);
                vertMap[t.c].erase(t);
            }

            bool containsSegment(Seg const &s) const
            {
                typename std::map<Vertex,std::set<Tri> >::const_iterator ts = vertMap.find(s.a);
                if(ts != vertMap.end())
                    for(typename std::set<Tri>::const_iterator t = ts->second.begin(); t != ts->second.end(); ++t)
                        if(t->contains(s))
                            return true;
                return false;
            }

            void InsertSegment(Seg const &s, Polygon<T> const &p)
            {
//                 if(!containsSegment(s))
//                     std::cerr << "InsertSegment: " << s.str(p) << std::endl;

                assert(vertMap.count(s.a) && vertMap.count(s.b));

                while(!containsSegment(s))
                {
                    bool found = false;
                    std::set<Tri> its = findIntersecting(s,p);
                    typename std::set<Tri>::const_iterator t1,t2;
                    for(t1 = its.begin(); t1 != its.end() && !found; ++t1)
                        for(t2 = its.begin(); t2 != its.end() && !found; ++t2)
                            if(t1 != t2)
                            {
                                bool pair = false;
                                if(t1->contains(t2->a))
                                {
                                    if(t1->contains(t2->b) || t1->contains(t2->c))
                                        pair = true;
                                }
                                else if(t1->contains(t2->b) && t1->contains(t2->c))
                                    pair = true;
                                if(pair)
                                {
                                    Seg ts = t1->shared(*t2);
                                    Vertex v1 = t1->getOppositeVertex(ts);
                                    Vertex v2 = t2->getOppositeVertex(ts);
                                    if(ts.intersects(Seg(v1,v2),p) && ts.intersects(s,p) && !Seg(v1,v2).intersects(s,p))
                                    {
                                        Tri nta(v1,ts.a,v2);
                                        Tri ntb(v1,ts.b,v2);
                                        erase(*t1);
                                        erase(*t2);
                                        insert(nta);
                                        insert(ntb);
                                        found = true;
                                    }
                                }
                            }
                    if(!found)
                        break;
                }
                if(!containsSegment(s))
                {
                    std::cerr << "InsertSegment: " << s.str(p,s.a) << std::endl;
                    std::cerr << "Segment NOT ADDED!" << std::endl;
                    std::set<Tri> its = findIntersecting(s,p);
                    for(typename std::set<Tri>::const_iterator t = its.begin(); t != its.end(); ++t)
                        std::cerr << "\t" << t->str(p,s.a) << std::endl;
                }
                //assert(containsSegment(s));
            }

            std::set<Tri> findIntersecting(Seg const &s, Polygon<T> const &p) const
            {
                std::set<Tri> ret;
                std::set<Vertex> seenlist;
                std::queue<Vertex> notseenlist;
                notseenlist.push(s.a);
                while(!notseenlist.empty())
                {
                    Vertex v = notseenlist.front();
                    notseenlist.pop();
                    if(seenlist.count(v) == 0)
                    {
                        std::set<Tri> subret = findIntersecting(v,s,p);
                        for(typename std::set<Tri>::const_iterator t = subret.begin(); t != subret.end(); ++t)
                        {
                            if (ret.count(*t) == 0)
                            {
                                ret.insert(*t);
                                if(!seenlist.count(t->a))
                                    notseenlist.push(t->a);
                                if(!seenlist.count(t->b))
                                    notseenlist.push(t->b);
                                if(!seenlist.count(t->c))
                                    notseenlist.push(t->c);
                            }
                        }
                        seenlist.insert(v);
                    }
                }
                return ret;
            }

            std::set<Tri> findIntersecting(Vertex const &v, Seg const &s, Polygon<T> const &p) const
            {
                std::set<Tri> ret;
                typename std::map<Vertex,std::set<Tri> >::const_iterator vi = vertMap.find(v);
                for(typename std::set<Tri>::const_iterator t = vi->second.begin(); t != vi->second.end(); ++t)
                {
                    if(Seg(t->a,t->b).intersects(s,p)||Seg(t->b,t->c).intersects(s,p)||Seg(t->a,t->c).intersects(s,p))
                        ret.insert(*t);
                }
                return ret;
            }

            Tri opposite(Seg const &s, Vertex const &v) const
            {
                typename std::map<Vertex,std::set<Tri> >::const_iterator ts = vertMap.find(s.a);
                if(ts != vertMap.end())
                    for(typename std::set<Tri>::iterator t = ts->second.begin(); t != ts->second.end(); ++t)
                        if(t->contains(s) && !t->contains(v))
                            return *t;
                return Tri();
            }

            Tri opposite(Tri const &t, Vertex const &v) const
            {
                return opposite(t.getOppositeSegment(v),v);
            }

            void DelaunayCheck(Tri const &t, Polygon<T> const &p)
            {
                //std::cerr << "DelaunayCheck: " << t.str(p) << std::endl;

                assert(triangles.count(t));
                //Circle<T> cc = t(p).circumcircle();
                //std::cerr << "\tcircumcircle: " << p.sw()-cc.center << " r: " << cc.radius << std::endl;

                Vertex vs[3] = {t.a,t.b,t.c};
                Seg segs[3] = {Seg(t.b,t.c), Seg(t.a,t.c), Seg(t.a,t.b)};
                typename T::value_type dv = 0.0;

                int dt = -1;

                for(int i = 0; i < 3; ++i)
                {
                    if(!segs[i].fixed(p))
                    {
                        Tri ct = opposite(t,vs[i]);
                        if(ct.valid())
                        {
                            //std::cerr << "\t" << "candidate triangle: " << ct.str(p) << std::endl;
                            Vertex cv = ct.getOppositeVertex(segs[i]);

                            if(segs[i].intersects(Seg(vs[i],cv),p))
                            {
                                typename T::value_type cdv = t(p).delaunay(cv(p));
                                if(cdv > dv)
                                {
                                    dv = cdv;
                                    dt = i;
                                }
                            }
                        }
                    }
                }

                if(dt >= 0)
                {
                    Tri ot = opposite(t,vs[dt]);
                    Vertex ov = ot.getOppositeVertex(segs[dt]);
                    //std::cerr << "\t" << ot.str() << "\t" << dv << std::endl;

                    Tri nta(vs[dt],segs[dt].a,ov);
                    Tri ntb(vs[dt],segs[dt].b,ov);

                    if(nta(p).delaunay(segs[dt].b(p))<dv && ntb(p).delaunay(segs[dt].a(p))<dv)
                    {
                        erase(t);
                        erase(ot);

                        insert(Tri(vs[dt],segs[dt].a,ov));
                        DelaunayCheck(Tri(vs[dt],segs[dt].a,ov),p);

                        insert(Tri(vs[dt],segs[dt].b,ov));
                        DelaunayCheck(Tri(vs[dt],segs[dt].b,ov),p);
                    }
                }
            }
        };

        typedef std::map<XYIndex,Vertex> VertexMap;
        typedef std::map<YXIndex,Vertex> FrontSet;

        void FillFront(FrontSet &front, Polygon<T> const &p, TriList &triangles)
        {
            typename FrontSet::iterator f = front.begin();
            while(f != front.end())
            {
                typename FrontSet::iterator f2 = f;
                ++f2;
                if(f2 != front.end())
                {
                    typename FrontSet::iterator f3 = f2;
                    ++f3;
                    if(f3 != front.end())
                    {
                        Triangle<T> t(f->second(p),f2->second(p),f3->second(p));
                        if(t.valid() && t.clockwise())
                        {
                            Tri tri(f->second,f2->second,f3->second);
                            triangles.insert(tri);
                            triangles.DelaunayCheck(tri,p);
                            front.erase(f2);
                            f = front.begin();
                        }
                        else
                            ++f;
                    }
                    else
                        f = front.end();
                }
                else
                    f = front.end();
            }
        }

        /// Based on "Sweep-line algorithm for constrained Delaunay triangulation" by Domiter and Zalik.
        void ConstrainedDelaunay(Polygon<T> const &pOriginal)
        {
            Polygon<T> p;
            std::set<XYIndex> vs;
            for(typename Polygon<T>::ring_type::const_iterator v = pOriginal.getExteriorRing().begin(); v != pOriginal.getExteriorRing().end(); ++v)
            {
                if(vs.count(*v)==0)
                {
                    p.insert(*v);
                    vs.insert(*v);
                }
            }
            for(int r = 0; r <  pOriginal.getInteriorRings().size(); ++r)
            {
                for(typename Polygon<T>::ring_type::const_iterator v = pOriginal.getInteriorRings()[r].begin(); v != pOriginal.getInteriorRings()[r].end(); ++v)
                {
                    if(vs.count(*v)==0)
                    {
                        p.insert(r,*v);
                        vs.insert(*v);
                    }
                }
            }

            if(!p.getExteriorRing().empty())
            {
                VertexMap verts;

                for(int i = 0; i < p.getExteriorRing().size(); ++i)
                {
                    if(verts.count(XYIndex(p.getExteriorRing()[i])))
                        std::cerr << Vertex(i).str() << " is a duplicate of " << verts[XYIndex(p.getExteriorRing()[i])].str() << std::endl;
                    verts.insert(verts.begin(), std::make_pair(XYIndex(p.getExteriorRing()[i]),Vertex(i)));
                }
                for(int r = 0; r < p.getInteriorRings().size(); ++r)
                    for(int i = 0; i < p.getInteriorRings()[r].size(); ++i)
                    {
                        if(i != p.getInteriorRings()[r].size()-1 || p.getInteriorRings()[r][i] != p.getInteriorRings()[r][0])
                        {
                            if(verts.count(XYIndex(p.getInteriorRings()[r][i])))
                                std::cerr << Vertex(r,i).str() << " is a duplicate of " << verts[XYIndex(p.getInteriorRings()[r][i])].str() << std::endl;
                            verts.insert(verts.begin(), std::make_pair(XYIndex(p.getInteriorRings()[r][i]),Vertex(r,i)));
                        }
                    }

                verts.insert(verts.begin(), std::make_pair(XYIndex(p.sw()),Vertex(-2,0)));
                verts.insert(verts.begin(), std::make_pair(XYIndex(p.nw()),Vertex(-2,1)));

                TriList triangles;
                FrontSet front;
                for(typename VertexMap::iterator sweepLine = verts.begin(); sweepLine != verts.end(); ++sweepLine)
                {
                    typename FrontSet::iterator fu1 = front.upper_bound(sweepLine->first);
                    typename FrontSet::reverse_iterator fd1(fu1);
                    if(fu1 != front.end() && fd1 != front.rend())
                    {
                        Tri tri(sweepLine->second,fu1->second,fd1->second);
                        assert(tri(p).valid());
                        triangles.insert(tri);
                        //std::cerr << "inserted " << sweepLine->second.str() << "\t" << tri.str() << std::endl;
                        triangles.DelaunayCheck(tri,p);
                    }
/*                    else
                        std::cerr << "skipping " << sweepLine->second.str() << std::endl;*/
                    front[sweepLine->first]=sweepLine->second;
                    FillFront(front,p,triangles);

                    Vertex v = sweepLine->second;
                    if(v.ring>=-1)
                    {
                        Vertex vn = v.next(p);
                        Vertex vp = v.prev(p);

                        if(triangles.vertMap.count(vn))
                            triangles.InsertSegment(Seg(v,vn),p);

                        if(triangles.vertMap.count(vp))
                            triangles.InsertSegment(Seg(v,vp),p);
                    }

                }

                for(typename std::set<Tri>::iterator t = triangles.triangles.begin(); t != triangles.triangles.end(); ++t)
                {
                    if(!t->a.isDummy() && !t->b.isDummy() && !t->c.isDummy())
                    {
                        if(!t->a.isInterior() && !t->b.isInterior() && !t->c.isInterior())
                        {
                            Triangle<T> tri(t->a(p),t->b(p),t->c(p));
                            if(!tri.clockwise())
                            {
                                vertices.push_back(tri.a);
                                vertices.push_back(tri.b);
                                vertices.push_back(tri.c);
                            }
                        }
                        else if(t->a.ring != t->b.ring || t->a.ring != t->c.ring)
                        {
                            Triangle<T> tri(t->a(p),t->b(p),t->c(p));
                            if(!tri.clockwise())
                            {
                                vertices.push_back(tri.a);
                                vertices.push_back(tri.b);
                                vertices.push_back(tri.c);
                            }
                            else
                            {
                                vertices.push_back(tri.a);
                                vertices.push_back(tri.c);
                                vertices.push_back(tri.b);
                            }
                        }
                        else
                        {
                            Triangle<T> tri(t->a(p),t->b(p),t->c(p));
                            if(tri.clockwise())
                            {
                                vertices.push_back(tri.a);
                                vertices.push_back(tri.c);
                                vertices.push_back(tri.b);
                            }
                        }
                    }
                }
            }
        }

        void EarClip(Polygon<T> const &p)
        {
            std::map<XYIndex,int> allPoints;
            std::set<int> availableVerts;
            for(int i = 0; i < p.getExteriorRing().size(); ++i)
            {
                availableVerts.insert(i);
                allPoints[p.getExteriorRing()[i]] = i;
            }

            int lastCount = 0;
            int vertCount = availableVerts.size();
            while(vertCount != lastCount)
            {
                lastCount = vertCount;
                //std::cerr << availableVerts.size() << " verts left" << std::endl;
                for(std::set<int>::const_iterator i = availableVerts.begin(); i != availableVerts.end(); ++i)
                {
                    std::set<int>::const_iterator i2 = i;
                    ++i2;
                    if(i2 == availableVerts.end())
                        i2 = availableVerts.begin();
                    std::set<int>::const_iterator i3 = i2;
                    ++i3;
                    if(i3 == availableVerts.end())
                        i3 = availableVerts.begin();
                    Triangle<T> pt(p.getExteriorRing()[*i],p.getExteriorRing()[*i2],p.getExteriorRing()[*i3]);
                    bool bad = !pt.valid() || pt.clockwise();
                    if(!bad)
                    {
                        typename std::map<XYIndex,int>::const_iterator b = allPoints.find(std::min(XYIndex(pt.a),std::min(XYIndex(pt.b),XYIndex(pt.c))));
                        typename std::map<XYIndex,int>::const_iterator e = allPoints.find(std::max(XYIndex(pt.a),std::max(XYIndex(pt.b),XYIndex(pt.c))));
                        
                        for(typename std::map<XYIndex,int>::const_iterator tpi = b; tpi != e && !bad; ++tpi)
                        {
                            T tp = p.getExteriorRing()[tpi->second];
                            if(tp != pt.a && tp != pt.b && tp != pt.c && pt.contains(tp))
                                bad = true;
                        }
                    }
                    if(!bad)
                    {
                        vertices.push_back(p.getExteriorRing()[*i]);
                        vertices.push_back(p.getExteriorRing()[*i2]);
                        vertices.push_back(p.getExteriorRing()[*i3]);
                        availableVerts.erase(*i2);
                        --vertCount;
                        break;
                    }
                }
            }
            std::cerr << availableVerts.size() << std::endl;
            for(std::set<int>::const_iterator i = availableVerts.begin(); i != availableVerts.end(); ++i)
                std::cerr << "\t" << *i << "\t" << p.getExteriorRing()[*i] << std::endl;
        }

        TriangulatedPolygon(Polygon<T> const &p)
        {
            //ConstrainedDelaunay(p);
            EarClip(p);

        }
    };
}


#endif
