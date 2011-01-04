#include "AtomSpaceAsync.h"
#include "TimeServer.h"

using namespace opencog;

AtomSpaceAsync::AtomSpaceAsync()
{
    pthread_mutex_init(&atomSpaceLock, NULL); 
    processingRequests = false;
    counter = 0;
    spaceServer = new SpaceServer(*this);
    timeServer = new TimeServer(*this,spaceServer);
    atomspace.setSpaceServer(spaceServer);
    // Start event loop
    startEventLoop();
}

AtomSpaceAsync::~AtomSpaceAsync()
{
    stopEventLoop();
    delete timeServer;
    delete spaceServer;
};

void AtomSpaceAsync::startEventLoop()
{
    processingRequests = true;
    m_Thread = boost::thread(&AtomSpaceAsync::eventLoop, this);
}

void AtomSpaceAsync::stopEventLoop()
{
    // Tell request worker thread to exit
    processingRequests=false;
    requestQueue.cancel();
    // rejoin thread
    m_Thread.join();
}

void AtomSpaceAsync::eventLoop()
{
    try {
        while (processingRequests) {
            boost::shared_ptr<ASRequest> req;
            requestQueue.wait_and_pop(req);
            counter++;
            req->run();
        }
    } catch (concurrent_queue< boost::shared_ptr<ASRequest> >::Canceled &e) {
        //cout << "End AtomSpace event loop" << endl;
        return;
    }
}

