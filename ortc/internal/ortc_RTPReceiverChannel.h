/*

 Copyright (c) 2015, Hookflash Inc. / Hookflash Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.
 
 */

#pragma once

#include <ortc/internal/types.h>
#include <ortc/internal/ortc_ISecureTransport.h>

#include <ortc/IICETransport.h>
#include <ortc/IDTLSTransport.h>
#include <ortc/IRTPTypes.h>

#include <openpeer/services/IWakeDelegate.h>
#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/Timer.h>


//#define ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE "ortc/sctp/max-message-size"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverChannelForSettings)
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverChannelForRTPReceiver)
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverChannelForMediaStreamTrack)

    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverForRTPReceiverChannel)
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPReceiverChannel)

    ZS_DECLARE_INTERACTION_PROXY(IRTPReceiverChannelAsyncDelegate)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelForSettings
    #pragma mark

    interaction IRTPReceiverChannelForSettings
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelForSettings, ForSettings)

      static void applyDefaults();

      virtual ~IRTPReceiverChannelForSettings() {}
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelForRTPReceiver
    #pragma mark

    interaction IRTPReceiverChannelForRTPReceiver
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelForRTPReceiver, ForRTPReceiver)

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)
      typedef std::list<RTCPPacketPtr> RTCPPacketList;
      ZS_DECLARE_PTR(RTCPPacketList)

      static ElementPtr toDebug(ForRTPReceiverPtr object);

      static RTPReceiverChannelPtr create(
                                          RTPReceiverPtr receiver,
                                          const Parameters &params,
                                          const RTCPPacketList &packets
                                          );

      virtual PUID getID() const = 0;

      virtual void notifyTransportState(ISecureTransport::States state) = 0;

      virtual void notifyPacket(RTPPacketPtr packet) = 0;

      virtual void notifyPackets(RTCPPacketListPtr packets) = 0;

      virtual void notifyUpdate(const Parameters &params) = 0;

      virtual bool handlePacket(RTPPacketPtr packet) = 0;

      virtual bool handlePacket(RTCPPacketPtr packet) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelForMediaStreamTrack
    #pragma mark

    interaction IRTPReceiverChannelForMediaStreamTrack
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelForMediaStreamTrack, ForMediaStreamTrack)

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      static ElementPtr toDebug(ForMediaStreamTrackPtr object);

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelAsyncDelegate
    #pragma mark

    interaction IRTPReceiverChannelAsyncDelegate
    {
      typedef std::list<RTCPPacketPtr> RTCPPacketList;
      ZS_DECLARE_PTR(RTCPPacketList)
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      virtual void onSecureTransportState(ISecureTransport::States state) = 0;

      virtual void onNotifyPacket(RTPPacketPtr packet) = 0;

      virtual void onNotifyPackets(RTCPPacketListPtr packets) = 0;

      virtual void onUpdate(ParametersPtr params) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannel
    #pragma mark
    
    class RTPReceiverChannel : public Noop,
                               public MessageQueueAssociator,
                               public SharedRecursiveLock,
                               public IRTPReceiverChannelForSettings,
                               public IRTPReceiverChannelForRTPReceiver,
                               public IRTPReceiverChannelForMediaStreamTrack,
                               public IWakeDelegate,
                               public zsLib::ITimerDelegate,
                               public IRTPReceiverChannelAsyncDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IRTPReceiverChannel;
      friend interaction IRTPReceiverChannelFactory;
      friend interaction IRTPReceiverChannelForSettings;
      friend interaction IRTPReceiverChannelForRTPReceiver;
      friend interaction IRTPReceiverChannelForMediaStreamTrack;

      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverForRTPReceiverChannel, UseReceiver)
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPReceiverChannel, UseMediaStreamTrack)
      
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)
      typedef std::list<RTCPPacketPtr> RTCPPacketList;
      ZS_DECLARE_PTR(RTCPPacketList)

      enum States
      {
        State_Pending,
        State_Ready,
        State_ShuttingDown,
        State_Shutdown,
      };
      static const char *toString(States state);

    public:
      RTPReceiverChannel(
                         const make_private &,
                         IMessageQueuePtr queue,
                         UseReceiverPtr receiver,
                         const Parameters &params
                         );

    protected:
      RTPReceiverChannel(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init(const RTCPPacketList &packets);

    public:
      virtual ~RTPReceiverChannel();

      static RTPReceiverChannelPtr convert(ForSettingsPtr object);
      static RTPReceiverChannelPtr convert(ForRTPReceiverPtr object);
      static RTPReceiverChannelPtr convert(ForMediaStreamTrackPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannel => IRTPReceiverChannelForRTPReceiver
      #pragma mark

      static ElementPtr toDebug(RTPReceiverChannelPtr object);

      static RTPReceiverChannelPtr create(
                                          RTPReceiverPtr receiver,
                                          const Parameters &params,
                                          const RTCPPacketList &packets
                                          );

      virtual PUID getID() const override {return mID;}

      virtual void notifyTransportState(ISecureTransport::States state) override;

      virtual void notifyPacket(RTPPacketPtr packet) override;

      virtual void notifyPackets(RTCPPacketListPtr packets) override;

      virtual void notifyUpdate(const Parameters &params) override;

      virtual bool handlePacket(RTPPacketPtr packet) override;

      virtual bool handlePacket(RTCPPacketPtr packet) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannel => IRTPReceiverChannelForMediaStreamTrack
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForMediaStreamTrackPtr object);

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannel => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannel => ITimerDelegate
      #pragma mark

      virtual void onTimer(TimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannel => IRTPReceiverChannelAsyncDelegate
      #pragma mark

      virtual void onSecureTransportState(ISecureTransport::States state) override;

      virtual void onNotifyPacket(RTPPacketPtr packet) override;

      virtual void onNotifyPackets(RTCPPacketListPtr packets) override;

      virtual void onUpdate(ParametersPtr params) override;

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannel => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();
      bool stepBogusDoSomething();

      void cancel();

      void setState(States state);
      void setError(WORD error, const char *reason = NULL);


    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannel => (data)
      #pragma mark

      AutoPUID mID;
      RTPReceiverChannelWeakPtr mThisWeak;
      RTPReceiverChannelPtr mGracefulShutdownReference;


      States mCurrentState {State_Pending};

      WORD mLastError {};
      String mLastErrorReason;

      ISecureTransport::States mSecureTransportState {ISecureTransport::State_Pending};

      UseReceiverWeakPtr mReceiver;

      ParametersPtr mParameters;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelFactory
    #pragma mark

    interaction IRTPReceiverChannelFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)
      typedef std::list<RTCPPacketPtr> RTCPPacketList;

      static IRTPReceiverChannelFactory &singleton();

      virtual RTPReceiverChannelPtr create(
                                           RTPReceiverPtr receiver,
                                           const Parameters &params,
                                           const RTCPPacketList &packets
                                           );
    };

    class RTPReceiverChannelFactory : public IFactory<IRTPReceiverChannelFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IRTPReceiverChannelAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::ISecureTransport::States, States)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IRTPReceiverChannelAsyncDelegate::RTCPPacketListPtr, RTCPPacketListPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::RTPPacketPtr, RTPPacketPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::IRTPReceiverChannelAsyncDelegate::ParametersPtr, ParametersPtr)
ZS_DECLARE_PROXY_METHOD_1(onSecureTransportState, States)
ZS_DECLARE_PROXY_METHOD_1(onNotifyPacket, RTPPacketPtr)
ZS_DECLARE_PROXY_METHOD_1(onNotifyPackets, RTCPPacketListPtr)
ZS_DECLARE_PROXY_METHOD_1(onUpdate, ParametersPtr)
ZS_DECLARE_PROXY_END()

