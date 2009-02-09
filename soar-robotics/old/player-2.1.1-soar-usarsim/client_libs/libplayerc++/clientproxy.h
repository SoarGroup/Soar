/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000-2003
 *     Brian Gerkey, Kasper Stoy, Richard Vaughan, & Andrew Howard
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/********************************************************************
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ********************************************************************/

/***************************************************************************
 * Desc: Player v2.0 C++ client
 * Authors: Brad Kratochvil, Toby Collett
 *
 * Date: 23 Sep 2005
 # CVS: $Id: clientproxy.h 6558 2008-06-13 23:37:38Z thjc $
 **************************************************************************/


#ifndef PLAYERCC_CLIENTPROXY_H
#define PLAYERCC_CLIENTPROXY_H
namespace PlayerCc
{

/** @brief The client proxy base class
 *
 * Base class for all proxy devices. Access to a device is provided by a
 * device-specific proxy class.  These classes all inherit from the @p
 * ClientProxy class which defines an interface for device proxies.  As such,
 * a few methods are common to all devices and we explain them here.
 *
 * Since the ConnectReadSignal() and DisconnectReadSignal() member functions
 * are based on the Boost signals library, they are conditionally available
 * depending on Boost's presence in the system.  See the @em configure script
 * for more information.
*/
class ClientProxy
{
  friend class PlayerClient;

  public:

#ifdef HAVE_BOOST_SIGNALS
    /// A connection type.  This is usefull when attaching signals to the
    /// ClientProxy because it allows for detatching the signals.
    typedef boost::signals::connection connection_t;

    /// A scoped lock
    typedef boost::mutex::scoped_lock scoped_lock_t;

    /// the function pointer type for read signals signal
    typedef boost::signal<void (void)> read_signal_t;
#else
    // if we're not using boost, just define them.
    typedef int connection_t;
    // we redefine boost::mustex::scoped_lock in playerclient.h
    typedef boost::mutex::scoped_lock scoped_lock_t;
    // if we're not using boost, just define them.
    typedef int read_signal_t;
#endif

  protected:

    // The ClientProxy constructor
    // @attention Potected, so it can only be instantiated by other clients
    //
    // @throw PlayerError Throws a PlayerError if unable to connect to the client
    ClientProxy(PlayerClient* aPc, uint32_t aIndex);

    // destructor will try to close access to the device
    virtual ~ClientProxy();

    // Subscribe to the proxy
    // This needs to be defined for every proxy.
    // @arg aIndex the index of the devce we want to connect to

    // I wish these could be pure virtual,
    // but they're used in the constructor/destructor
    virtual void Subscribe(uint32_t /*aIndex*/) {};

    // Unsubscribe from the proxy
    // This needs to be defined for every proxy.
    virtual void Unsubscribe() {};

    // The controlling client object.
    PlayerClient* mPc;

    // A reference to the C client
    playerc_client_t* mClient;

    // contains convenience information about the device
    playerc_device_t *mInfo;

    // if set to true, the current data is "fresh"
    bool mFresh;

    // @brief Get a variable from the client
    // All Get functions need to use this when accessing data from the
    // c library to make sure the data access is thread safe.
    template<typename T>
    T GetVar(const T &aV) const
    { // these have to be defined here since they're templates
      scoped_lock_t lock(mPc->mMutex);
      T v = aV;
      return v;
    }

    // @brief Get a variable from the client by reference
    // All Get functions need to use this when accessing data from the
    // c library to make sure the data access is thread safe.  In this
    // case, a begin, end, and destination pointer must be given (similar
    // to C++ copy).  It is up to the user to ensure there is memory
    // allocated at aDest.
    template<typename T>
    void GetVarByRef(const T aBegin, const T aEnd, T aDest) const
    { // these have to be defined here since they're templates
      scoped_lock_t lock(mPc->mMutex);
      std::copy(aBegin, aEnd, aDest);
    }

  private:

    // The last time that data was read by this client in [s].
    double mLastTime;

    // A boost::signal which is used for our callbacks.
    // The signal will normally be of a type such as:
    // - boost::signal<void ()>
    // - boost::signal<void (T)>
    // where T can be any type.
    //
    // @attention we currently only use signals that return void because we
    // don't have checks to make sure a signal is registered.  If an empty
    // signal is called:
    //
    // @attention "Calling the function call operator may invoke undefined
    // behavior if no slots are connected to the signal, depending on the
    // combiner used. The default combiner is well-defined for zero slots when
    // the return type is void but is undefined when the return type is any
    // other type (because there is no way to synthesize a return value)."
    //
    read_signal_t mReadSignal;

    // Outputs the signal if there is new data
    void ReadSignal();

  public:

    ///  Returns true if we have received any data from the device.
    bool IsValid() const { return 0!=GetVar(mInfo->datatime); };

    /// Fresh is set to true on each new read.  It is up to the user to
    /// set it to false if the data has already been read.  This is most
    /// useful when used in conjunction with the PlayerMultiClient
    bool IsFresh() const { return GetVar(mFresh); };

    /// This states that the data in a client is currently not Fresh
    void NotFresh();

    ///  Returns the driver name
    ///  @todo GetDriverName isn't guarded by locks yet
    std::string GetDriverName() const { return mInfo->drivername; };

    /// Returns the received timestamp [s]
    double GetDataTime() const { return GetVar(mInfo->datatime); };

    /// Returns the received timestamp [s]
    double GetElapsedTime() const
      { return GetVar(mInfo->datatime) - GetVar(mInfo->lasttime); };

    /// Returns a pointer to the Player Client
    PlayerClient * GetPlayerClient() const { return mPc;}
    /// Returns device index
    uint32_t GetIndex() const { return GetVar(mInfo->addr.index); };

    /// Returns device interface
    uint32_t GetInterface() const { return GetVar(mInfo->addr.interf); };

    /// Returns device interface
    std::string GetInterfaceStr() const
      { return interf_to_str(GetVar(mInfo->addr.interf)); };

    /// @brief Set a replace rule for this proxy on the server.
    ///
    /// If a rule with the same pattern already exists, it will be replaced
    /// with the new rule (i.e., its setting to replace will be updated).
    /// @param aReplace Should we replace these messages
    /// @param aType The type to set replace rule for (-1 for wildcard),
    ///        see @ref message_types.
    /// @param aSubtype Message subtype to set replace rule for (-1 for
    ///        wildcard).  This is dependent on the @ref interfaces.
    ///
    /// @exception throws PlayerError if unsuccessfull
    ///
    /// @see PlayerClient::SetReplaceRule, PlayerClient::SetDataMode
    void SetReplaceRule(bool aReplace,
                        int aType = -1,
                        int aSubtype = -1);

    /// @brief Request capabilities of device.
    ///
    /// Send a message asking if the device supports the given message
    /// type and subtype. If it does, the return value will be 1, and 0 otherwise.
    int HasCapability(uint32_t aType, uint32_t aSubtype);

    /// @brief Request an integer property
    int GetIntProp(char *aProperty, int32_t *aValue);

    /// @brief Set an integer property
    int SetIntProp(char *aProperty, int32_t aValue);

    /// @brief Request a double property
    int GetDblProp(char *aProperty, double *aValue);

    /// @brief Set a double property
    int SetDblProp(char *aProperty, double aValue);

    /// @brief Request a string property
    int GetStrProp(char *aProperty, char **aValue);

    /// @brief Set a string property
    int SetStrProp(char *aProperty, char *aValue);

    /// Connect a signal to this proxy
    /// For more information check out @ref player_clientlib_multi
    template<typename T>
    connection_t ConnectReadSignal(T aSubscriber)
      {
#ifdef HAVE_BOOST_SIGNALS
        scoped_lock_t lock(mPc->mMutex);
        return mReadSignal.connect(aSubscriber);
#else
        return -1;
#endif
      }

    /// Disconnect a signal to this proxy
    void DisconnectReadSignal(connection_t aSubscriber)
      {
#ifdef HAVE_BOOST_SIGNALS
        scoped_lock_t lock(mPc->mMutex);
        aSubscriber.disconnect();
#else
       // This line is here to prevent compiler warnings of "unused varaibles"
       aSubscriber = aSubscriber;
#endif
      }

};

}// namespace

#endif
