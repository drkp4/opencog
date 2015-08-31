/*
 * Copyright (C) 2008 by OpenCog Foundation
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <time.h>
#include <sstream>

#include <opencog/server/CogServer.h>
#include <opencog/atomspace/AtomSpace.h>

#include "TulipWriter.h"

using std::endl;

namespace opencog {


void TulipWriter::writeNodes()
{
    AtomSpace& a = BaseServer::getAtomSpace();

    HandleSeq nodeHandles;
    a.get_handles_by_type(back_inserter(nodeHandles), (Type) NODE, true );
    // write nodes for links too (to represent hypergraph in Tulip)
    HandleSeq linkHandles;
    a.get_handles_by_type(back_inserter(linkHandles), (Type) LINK, true );

    // Output Node numbers/ids
    myfile << "(nodes ";
    for (Handle h : nodeHandles) {
        myfile << h << " ";
    }
    for (Handle h : linkHandles) {
        myfile << h << " ";
    }
    myfile << ")" << endl;

}

void TulipWriter::writeHeader(std::string comment)
{
    // Write header
    myfile << "(tlp \"2.0\"" << endl;
    myfile << "(date \"" << getDateString() << "\")" << endl;
    myfile << "(comments \"This file was generated by OpenCog "
        "( http://opencog.org/ ). " + comment + "\")" << endl;
}

void TulipWriter::writeCluster(Handle setLink)
{
    AtomSpace& a = BaseServer::getAtomSpace();

    HandleSeq nodeHandles;
    a.get_handles_by_type(back_inserter(nodeHandles), (Type) NODE, true );
    HandleSeq linkHandles;
    a.get_handles_by_type(back_inserter(linkHandles), (Type) LINK, true );

    // Output setLink as a cluster
    std::set<Handle> inSet;
    if (setLink != Handle::UNDEFINED) {
        HandleSeq setLinks = a.get_outgoing(setLink);
        for (Handle h : setLinks) {
            inSet.insert(h);
        }
    }
    myfile << "(cluster 1 \"In Set\"" << endl;
    myfile << " (nodes ";
    for (Handle h : inSet) {
        myfile << h << " ";
    }
    myfile << ")\n)" << endl;

    // Output everything not in setLink as a cluster
    myfile << "(cluster 2 \"Not in set\"" << endl;
    myfile << " (nodes ";
    for (Handle h : nodeHandles) {
        std::set<Handle>::iterator si = inSet.find(h);
        if (si == inSet.end()) myfile << h << " ";
    }
    for (Handle h : linkHandles) {
        inSet.find(h);
    }
    myfile << ")" << endl;

    // TODO : also output the appropriate fake edges
//    myfile << " (edges ";
//    for (Handle h : linkHandles) {
//        std::set<Handle>::iterator si = inSet.find(h);
//        if (si == inSet.end()) {
//            myfile << h << " ";
//        }
//    }
//    myfile << ")
    myfile << "\n)" << endl;
}

void TulipWriter::writeEdges()
{
    AtomSpace& a = BaseServer::getAtomSpace();

    HandleSeq nodeHandles;
    a.get_handles_by_type(back_inserter(nodeHandles), (Type) NODE, true );
    HandleSeq linkHandles;
    a.get_handles_by_type(back_inserter(linkHandles), (Type) LINK, true );

    // Output Edge numbers/ids, source, and target
    for (Handle l : linkHandles) {
        // get outgoing set, for each destination add a link
        HandleSeq out = a.get_outgoing(l);
        for (Handle d : out) {
            myfile << "(edge " << l << d << " " << l << " " << d;
            myfile << ")" << endl;
        }
    }
}

void TulipWriter::writeNodeNames()
{
    AtomSpace& a = BaseServer::getAtomSpace();

    // Including type of link nodes
    HandleSeq nodeHandles;
    a.get_handles_by_type(back_inserter(nodeHandles), (Type) NODE, true );
    HandleSeq linkHandles;
    a.get_handles_by_type(back_inserter(linkHandles), (Type) LINK, true );

    // Output node names
    myfile << "(property  0 string \"viewLabel\" " << endl;
    myfile << "  (default \"\" \"\" )" << endl;
    for (Handle h : nodeHandles) {
        myfile << "  (node " << h << " \"" << a.get_name(h) << "\")" << endl;
    }
    // give not nodes the name NOT
    for (Handle h : linkHandles) {
        myfile << "(node " << h << " \"" << classserver().getTypeName(a.get_type(h)) 
            << "\" )" << endl;
    }
    myfile << ")" << endl;
}

void TulipWriter::writeDefaultColouring() {
    // Define default colouring
    myfile << "(property  0 color \"viewColor\"" << endl;
    myfile << "(default \"(35,0,235,255)\" \"(0,0,0,128)\" )" << endl;
    //for (Handle h : notLinks) {
    //    myfile << "(node " << h << " \"(235,0,35,255)\" )" << endl;
    //    myfile << "(edge " << h + notLinkOffsetIndex << " \"(235,35,35,255)\" )" << endl;
    //}
    myfile << ")" << endl;
}

void TulipWriter::writeTruthValue()
{
    AtomSpace& a = BaseServer::getAtomSpace();

    HandleSeq handles;
    a.get_handles_by_type(back_inserter(handles), (Type) ATOM, true );

    // Output strength component of truth value
    myfile << "(property  0 double \"strength\"" << endl;
    myfile << "(default \"0.0\" \"0.0\" )" << endl;
    for (Handle h : handles) {
        myfile << "  (node " << h << " \"" << a.get_mean(h) << "\")" << endl;
    }
    myfile << ")" << endl;

    HandleSeq nodeHandles;
    a.get_handles_by_type(back_inserter(nodeHandles), (Type) NODE, true );
    HandleSeq linkHandles;
    a.get_handles_by_type(back_inserter(nodeHandles), (Type) LINK, true );

    // Output distance metric as 1/strength 
    myfile << "(property  0 double \"distance\"" << endl;
    myfile << "(default \"0.0\" \"0.0\" )" << endl;
    for (Handle h : linkHandles) {
        // get outgoing set, for each destination add a link
        HandleSeq out = a.get_outgoing(h);
        for (Handle d : out) {
            myfile << "(edge " << h << d << " \"" << 1.0 / (a.get_mean(h)+0.0000001) << "\")" << endl;
        }
    }
    myfile << ")" << endl;

    // Output count component of truth value
    myfile << "(property  0 double \"count\"" << endl;
    myfile << "(default \"0.0\" \"0.0\" )" << endl;
    for (Handle h : handles) {
        myfile << "  (node " << h << " \"" << a.get_confidence(h) << "\")" << endl;
    }
    myfile << ")" << endl;
   
}

void TulipWriter::writeShapes()
{
    AtomSpace& a = BaseServer::getAtomSpace();

    HandleSeq nodeHandles;
    a.get_handles_by_type(back_inserter(nodeHandles), (Type) NODE, true );
    HandleSeq linkHandles;
    a.get_handles_by_type(back_inserter(nodeHandles), (Type) LINK, true );

    // Output strength component of truth value
    myfile << "(property  0 int \"viewShape\"" << endl;
    myfile << "(default \"0\" \"1\" )" << endl;
    for (Handle h : linkHandles) {
        myfile << " (node " << h << " \"1\" )" << endl;
    }
    myfile << ")" << endl;

}

bool TulipWriter::write(Handle seed, int depth, Handle setLink)
{
    //CogServer& cogserver = static_cast<CogServer&>(server());
    //AtomSpace* a = BaseServer::getAtomSpace();
    myfile.open( filename.c_str() );
    //int notLinkOffsetIndex = 1000000;

    writeHeader("");

    //HandleSeq nodesToWrite;
    //copy(nodeHandles.begin(), nodeHandles.end(), back_inserter(nodesToWrite));
    //copy(notLinks.begin(), notLinks.end(), back_inserter(nodesToWrite));
    //writeNodes(nodesToWrite);

    writeNodes();
    writeEdges();
    if (setLink != Handle::UNDEFINED) writeCluster(setLink);
    writeNodeNames();
    writeDefaultColouring();

    writeTruthValue();
    writeShapes();
   
    // Close header
    myfile << ")" << endl;
    myfile.close();

    return true;
}

std::string TulipWriter::getDateString()
{
    time_t rawtime;
    struct tm * timeinfo;
    std::ostringstream datestr;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    datestr << timeinfo->tm_mday << "-";
    datestr << timeinfo->tm_mon+1 << "-";
    datestr << timeinfo->tm_year + 1900;
    return datestr.str();

}

} // namespace opencog
