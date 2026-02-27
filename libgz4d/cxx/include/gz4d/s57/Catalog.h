#ifndef GZ4D_S57_CATALOG_H
#define GZ4D_S57_CATALOG_H


#include <vector>
#include <set>
#include <map>

#include <gz4d/gz4d.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <gz4d/Box.h>
#include <gz4d/vector/ios.h>
#include <gz4d/geo/TileSet.h>

namespace gz4d
{
    namespace s57
    {
        class Catalog
        {
        public:
            class Entry
            {
            public:
                typedef std::vector<gz4d::Box2d> BoxList;
            private:
                std::string title, alternateTitle;
                uint32_t resolution;
                BoxList bboxes;
            public:
                Entry(Entry const &e):title(e.title),alternateTitle(e.alternateTitle),resolution(e.resolution),bboxes(e.bboxes){}
                
                Entry(const boost::property_tree::ptree &n):resolution(0)
                {
                    //TODO: reimplement using ptree
                    
//                     boost::property_tree::ptree metadata = n.GetFirstNode().GetFirstNode().GetFirstNode();
//                     boost::property_tree::ptree mn = metadata.GetFirstNode("identificationInfo");
//                     if(mn)
//                     {
//                         boost::property_tree::ptree id = mn.GetFirstNode();
//                         
//                         boost::property_tree::ptree citation = id.GetFirstNode("citation");
//                         citation.FirstNode();
//                         boost::property_tree::ptree tn = citation.GetFirstNode("title");
//                         title = tn.GetFirstNode().Value();
//                         tn = citation.GetFirstNode("alternateTitle");
//                         alternateTitle = tn.GetFirstNode().Value();
// 
//                         boost::property_tree::ptree sr = id.GetFirstNode("spatialResolution");
//                         resolution = sr.GetFirstNode().GetFirstNode().GetFirstNode().GetFirstNode().GetFirstNode().Value<uint32_t>();
// 
//                         for(boost::property_tree::ptree ex = id.GetFirstNode("extent"); ex; ex.Next("extent"))
//                         {
//                             boost::property_tree::ptree pos = ex.GetFirstNode().GetFirstNode("geographicElement").GetFirstNode().GetFirstNode().GetFirstNode().GetFirstNode().GetFirstNode().GetFirstNode();
//                             gz4d::Box2d bbox;
//                             while(pos)
//                             {
//                                 std::stringstream pos_ss(pos.Value());
//                                 double lat,lon;
//                                 pos_ss >> lat >> lon;
//                                 bbox.expand(gz4d::Vector<double,2>(lon,lat));
//                                 pos.Next();
//                             }
//                             bboxes.push_back(bbox);
//                         }
//                     }
                }

                operator std::string() const
                {
                    std::stringstream ret;
                    ret << title << ", 1:" << resolution << "\t" << alternateTitle;
                    for(std::vector<Box2d>::const_iterator i = bboxes.begin(); i != bboxes.end(); ++i)
                        ret << "\n\t" << i->getMin() << " - " << i->getMax();
                    return ret.str();
                }

                uint32_t getResolution() const {return resolution;}
                BoxList const &getBoundingBoxes() const {return bboxes;}
                

                bool intersects(Box2d const &b) const
                {
                    for(std::vector<Box2d>::const_iterator i = bboxes.begin(); i != bboxes.end(); ++i)
                        if(i->intersects(b))
                            return true;
                    return false;
                }
            };

        private:
            std::vector<Entry> entries;

            typedef std::set<uint32_t> index_set;
            typedef geo::TileSet<index_set> TileIndexType;
            TileIndexType tileIndex;


        public:

            Catalog(std::string const &fname)
            {
                boost::property_tree::ptree catalog;
                read_xml(fname,catalog);
                
                boost::property_tree::ptree::const_iterator n = catalog.begin();
                for(boost::property_tree::ptree::const_iterator cn = n->second.begin(); cn != n->second.end(); ++cn)
                {
                    if(cn->first == "composedOf")
                        entries.push_back(Entry(cn->second));
                }
                
                for(uint32_t i = 0; i < entries.size(); ++i)
                {
                    std::cout << std::string(entries[i]) << std::endl;
                    int level = TileIndexType::level(entries[i].getResolution());
                    std::cout << "level: " << level << std::endl;
                    for(Entry::BoxList::const_iterator b = entries[i].getBoundingBoxes().begin(); b != entries[i].getBoundingBoxes().end(); ++b)
                    {
                        TileIndexType::index_type i1(level,b->getMin());
                        std::cout << "scale: " << i1.scale() << std::endl;
                        TileIndexType::index_type i2(level,b->getMax());
                        TileIndexType::index_type::range_type r(i1,i2);
                        std::cout << "index range: " << r._box.getMin() << " to " << r._box.getMax() << " size: " << r._box.getSizes() << std::endl;
                        for(TileIndexType::index_type::range_type::const_iterator ii = r.begin(); ii != r.end(); ++ii)
                        {
                            TileIndexType::node_type n = tileIndex(*ii);
                            n().insert(i);
                        }
                    }
                }
                std::cout << "tile index size: " << tileIndex.size() << std::endl;
            }

            void printTileIndex() const
            {
                for(TileIndexType::const_iterator i = tileIndex.begin(); i != tileIndex.end(); ++i)
                {
                    std::cout << int(i->first._level) << "\t" << i->first._coordinates << "\t";
                    for(index_set::const_iterator is = i->second.begin(); is != i->second.end(); ++is)
                        std::cout << *is << ",";
                    std::cout << std::endl;
                }
            }
        };
    }
}

#endif
