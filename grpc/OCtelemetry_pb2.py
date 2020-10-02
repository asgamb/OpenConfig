# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: OCtelemetry.proto

import sys
_b=sys.version_info[0]<3 and (lambda x:x) or (lambda x:x.encode('latin1'))
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor.FileDescriptor(
  name='OCtelemetry.proto',
  package='Telemetry',
  syntax='proto3',
  serialized_options=None,
  serialized_pb=_b('\n\x11OCtelemetry.proto\x12\tTelemetry\"\xe9\x01\n\x13SubscriptionRequest\x12\x19\n\x11observation_point\x18\x01 \x01(\t\x12\x13\n\x0bsuppression\x18\x02 \x01(\x08\x12\x10\n\x08interval\x18\x03 \x01(\r\x12\x10\n\x08\x64uration\x18\x04 \x01(\r\x12\x13\n\x0btemplate_id\x18\x05 \x01(\r\x12\x17\n\x0fsubscription_id\x18\x06 \x01(\x04\x12(\n\ncollectors\x18\x07 \x03(\x0b\x32\x14.Telemetry.Collector\x12&\n\tresources\x18\x08 \x03(\x0b\x32\x13.Telemetry.Resource\"\x0b\n\tNoMessage\"\x17\n\x07Message\x12\x0c\n\x04mesg\x18\x01 \x01(\t\"-\n\tCollector\x12\x12\n\nip_address\x18\x01 \x01(\t\x12\x0c\n\x04port\x18\x02 \x01(\r\"X\n\x08Resource\x12\x1d\n\x04path\x18\x01 \x01(\x0b\x32\x0f.Telemetry.Path\x12\x13\n\x0bpath_filter\x18\x02 \x01(\t\x12\x18\n\x10sample_frequency\x18\x03 \x01(\x03\"y\n\x14SubscriptionResponse\x12%\n\x02id\x18\x01 \x01(\x0b\x32\x19.Telemetry.SubscriptionId\x12:\n\x12\x61\x63tualSubscription\x18\x02 \x01(\x0b\x32\x1e.Telemetry.SubscriptionRequest\"\x1c\n\x0eSubscriptionId\x12\n\n\x02id\x18\x01 \x01(\r\"Q\n\x08KeyValue\x12\x0b\n\x03key\x18\x01 \x01(\t\x12\x11\n\tint_value\x18\x02 \x01(\x04\x12\x11\n\tstr_value\x18\x03 \x01(\t\x12\x12\n\nprefix_str\x18\x04 \x01(\t\"\xba\x01\n\rTelemetryData\x12\x11\n\ttimestamp\x18\x01 \x01(\x04\x12\x19\n\x11observation_point\x18\x02 \x01(\t\x12\x13\n\x0btemplate_id\x18\x03 \x01(\r\x12\x17\n\x0fsequence_number\x18\x04 \x01(\r\x12\x13\n\x0blast_sample\x18\x05 \x01(\x08\x12\x17\n\x0fsubscription_id\x18\x06 \x01(\x04\x12\x1f\n\x02kv\x18\x07 \x03(\x0b\x32\x13.Telemetry.KeyValue\"\x14\n\x04Path\x12\x0c\n\x04path\x18\x01 \x01(\t2\xb8\x01\n\x0bOCTelemetry\x12W\n\x12telemetrySubscribe\x12\x1e.Telemetry.SubscriptionRequest\x1a\x1f.Telemetry.SubscriptionResponse\"\x00\x12P\n\x1b\x63\x61ncelTelemetrySubscription\x12\x19.Telemetry.SubscriptionId\x1a\x14.Telemetry.NoMessage\"\x00\x32K\n\x07OCReply\x12@\n\nStreamData\x12\x18.Telemetry.TelemetryData\x1a\x14.Telemetry.NoMessage\"\x00(\x01\x62\x06proto3')
)




_SUBSCRIPTIONREQUEST = _descriptor.Descriptor(
  name='SubscriptionRequest',
  full_name='Telemetry.SubscriptionRequest',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='observation_point', full_name='Telemetry.SubscriptionRequest.observation_point', index=0,
      number=1, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='suppression', full_name='Telemetry.SubscriptionRequest.suppression', index=1,
      number=2, type=8, cpp_type=7, label=1,
      has_default_value=False, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='interval', full_name='Telemetry.SubscriptionRequest.interval', index=2,
      number=3, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='duration', full_name='Telemetry.SubscriptionRequest.duration', index=3,
      number=4, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='template_id', full_name='Telemetry.SubscriptionRequest.template_id', index=4,
      number=5, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='subscription_id', full_name='Telemetry.SubscriptionRequest.subscription_id', index=5,
      number=6, type=4, cpp_type=4, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='collectors', full_name='Telemetry.SubscriptionRequest.collectors', index=6,
      number=7, type=11, cpp_type=10, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='resources', full_name='Telemetry.SubscriptionRequest.resources', index=7,
      number=8, type=11, cpp_type=10, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=33,
  serialized_end=266,
)


_NOMESSAGE = _descriptor.Descriptor(
  name='NoMessage',
  full_name='Telemetry.NoMessage',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=268,
  serialized_end=279,
)


_MESSAGE = _descriptor.Descriptor(
  name='Message',
  full_name='Telemetry.Message',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='mesg', full_name='Telemetry.Message.mesg', index=0,
      number=1, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=281,
  serialized_end=304,
)


_COLLECTOR = _descriptor.Descriptor(
  name='Collector',
  full_name='Telemetry.Collector',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='ip_address', full_name='Telemetry.Collector.ip_address', index=0,
      number=1, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='port', full_name='Telemetry.Collector.port', index=1,
      number=2, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=306,
  serialized_end=351,
)


_RESOURCE = _descriptor.Descriptor(
  name='Resource',
  full_name='Telemetry.Resource',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='path', full_name='Telemetry.Resource.path', index=0,
      number=1, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='path_filter', full_name='Telemetry.Resource.path_filter', index=1,
      number=2, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='sample_frequency', full_name='Telemetry.Resource.sample_frequency', index=2,
      number=3, type=3, cpp_type=2, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=353,
  serialized_end=441,
)


_SUBSCRIPTIONRESPONSE = _descriptor.Descriptor(
  name='SubscriptionResponse',
  full_name='Telemetry.SubscriptionResponse',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='id', full_name='Telemetry.SubscriptionResponse.id', index=0,
      number=1, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='actualSubscription', full_name='Telemetry.SubscriptionResponse.actualSubscription', index=1,
      number=2, type=11, cpp_type=10, label=1,
      has_default_value=False, default_value=None,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=443,
  serialized_end=564,
)


_SUBSCRIPTIONID = _descriptor.Descriptor(
  name='SubscriptionId',
  full_name='Telemetry.SubscriptionId',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='id', full_name='Telemetry.SubscriptionId.id', index=0,
      number=1, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=566,
  serialized_end=594,
)


_KEYVALUE = _descriptor.Descriptor(
  name='KeyValue',
  full_name='Telemetry.KeyValue',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='key', full_name='Telemetry.KeyValue.key', index=0,
      number=1, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='int_value', full_name='Telemetry.KeyValue.int_value', index=1,
      number=2, type=4, cpp_type=4, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='str_value', full_name='Telemetry.KeyValue.str_value', index=2,
      number=3, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='prefix_str', full_name='Telemetry.KeyValue.prefix_str', index=3,
      number=4, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=596,
  serialized_end=677,
)


_TELEMETRYDATA = _descriptor.Descriptor(
  name='TelemetryData',
  full_name='Telemetry.TelemetryData',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='timestamp', full_name='Telemetry.TelemetryData.timestamp', index=0,
      number=1, type=4, cpp_type=4, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='observation_point', full_name='Telemetry.TelemetryData.observation_point', index=1,
      number=2, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='template_id', full_name='Telemetry.TelemetryData.template_id', index=2,
      number=3, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='sequence_number', full_name='Telemetry.TelemetryData.sequence_number', index=3,
      number=4, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='last_sample', full_name='Telemetry.TelemetryData.last_sample', index=4,
      number=5, type=8, cpp_type=7, label=1,
      has_default_value=False, default_value=False,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='subscription_id', full_name='Telemetry.TelemetryData.subscription_id', index=5,
      number=6, type=4, cpp_type=4, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
    _descriptor.FieldDescriptor(
      name='kv', full_name='Telemetry.TelemetryData.kv', index=6,
      number=7, type=11, cpp_type=10, label=3,
      has_default_value=False, default_value=[],
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=680,
  serialized_end=866,
)


_PATH = _descriptor.Descriptor(
  name='Path',
  full_name='Telemetry.Path',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='path', full_name='Telemetry.Path.path', index=0,
      number=1, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      serialized_options=None, file=DESCRIPTOR),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  serialized_options=None,
  is_extendable=False,
  syntax='proto3',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=868,
  serialized_end=888,
)

_SUBSCRIPTIONREQUEST.fields_by_name['collectors'].message_type = _COLLECTOR
_SUBSCRIPTIONREQUEST.fields_by_name['resources'].message_type = _RESOURCE
_RESOURCE.fields_by_name['path'].message_type = _PATH
_SUBSCRIPTIONRESPONSE.fields_by_name['id'].message_type = _SUBSCRIPTIONID
_SUBSCRIPTIONRESPONSE.fields_by_name['actualSubscription'].message_type = _SUBSCRIPTIONREQUEST
_TELEMETRYDATA.fields_by_name['kv'].message_type = _KEYVALUE
DESCRIPTOR.message_types_by_name['SubscriptionRequest'] = _SUBSCRIPTIONREQUEST
DESCRIPTOR.message_types_by_name['NoMessage'] = _NOMESSAGE
DESCRIPTOR.message_types_by_name['Message'] = _MESSAGE
DESCRIPTOR.message_types_by_name['Collector'] = _COLLECTOR
DESCRIPTOR.message_types_by_name['Resource'] = _RESOURCE
DESCRIPTOR.message_types_by_name['SubscriptionResponse'] = _SUBSCRIPTIONRESPONSE
DESCRIPTOR.message_types_by_name['SubscriptionId'] = _SUBSCRIPTIONID
DESCRIPTOR.message_types_by_name['KeyValue'] = _KEYVALUE
DESCRIPTOR.message_types_by_name['TelemetryData'] = _TELEMETRYDATA
DESCRIPTOR.message_types_by_name['Path'] = _PATH
_sym_db.RegisterFileDescriptor(DESCRIPTOR)

SubscriptionRequest = _reflection.GeneratedProtocolMessageType('SubscriptionRequest', (_message.Message,), dict(
  DESCRIPTOR = _SUBSCRIPTIONREQUEST,
  __module__ = 'OCtelemetry_pb2'
  # @@protoc_insertion_point(class_scope:Telemetry.SubscriptionRequest)
  ))
_sym_db.RegisterMessage(SubscriptionRequest)

NoMessage = _reflection.GeneratedProtocolMessageType('NoMessage', (_message.Message,), dict(
  DESCRIPTOR = _NOMESSAGE,
  __module__ = 'OCtelemetry_pb2'
  # @@protoc_insertion_point(class_scope:Telemetry.NoMessage)
  ))
_sym_db.RegisterMessage(NoMessage)

Message = _reflection.GeneratedProtocolMessageType('Message', (_message.Message,), dict(
  DESCRIPTOR = _MESSAGE,
  __module__ = 'OCtelemetry_pb2'
  # @@protoc_insertion_point(class_scope:Telemetry.Message)
  ))
_sym_db.RegisterMessage(Message)

Collector = _reflection.GeneratedProtocolMessageType('Collector', (_message.Message,), dict(
  DESCRIPTOR = _COLLECTOR,
  __module__ = 'OCtelemetry_pb2'
  # @@protoc_insertion_point(class_scope:Telemetry.Collector)
  ))
_sym_db.RegisterMessage(Collector)

Resource = _reflection.GeneratedProtocolMessageType('Resource', (_message.Message,), dict(
  DESCRIPTOR = _RESOURCE,
  __module__ = 'OCtelemetry_pb2'
  # @@protoc_insertion_point(class_scope:Telemetry.Resource)
  ))
_sym_db.RegisterMessage(Resource)

SubscriptionResponse = _reflection.GeneratedProtocolMessageType('SubscriptionResponse', (_message.Message,), dict(
  DESCRIPTOR = _SUBSCRIPTIONRESPONSE,
  __module__ = 'OCtelemetry_pb2'
  # @@protoc_insertion_point(class_scope:Telemetry.SubscriptionResponse)
  ))
_sym_db.RegisterMessage(SubscriptionResponse)

SubscriptionId = _reflection.GeneratedProtocolMessageType('SubscriptionId', (_message.Message,), dict(
  DESCRIPTOR = _SUBSCRIPTIONID,
  __module__ = 'OCtelemetry_pb2'
  # @@protoc_insertion_point(class_scope:Telemetry.SubscriptionId)
  ))
_sym_db.RegisterMessage(SubscriptionId)

KeyValue = _reflection.GeneratedProtocolMessageType('KeyValue', (_message.Message,), dict(
  DESCRIPTOR = _KEYVALUE,
  __module__ = 'OCtelemetry_pb2'
  # @@protoc_insertion_point(class_scope:Telemetry.KeyValue)
  ))
_sym_db.RegisterMessage(KeyValue)

TelemetryData = _reflection.GeneratedProtocolMessageType('TelemetryData', (_message.Message,), dict(
  DESCRIPTOR = _TELEMETRYDATA,
  __module__ = 'OCtelemetry_pb2'
  # @@protoc_insertion_point(class_scope:Telemetry.TelemetryData)
  ))
_sym_db.RegisterMessage(TelemetryData)

Path = _reflection.GeneratedProtocolMessageType('Path', (_message.Message,), dict(
  DESCRIPTOR = _PATH,
  __module__ = 'OCtelemetry_pb2'
  # @@protoc_insertion_point(class_scope:Telemetry.Path)
  ))
_sym_db.RegisterMessage(Path)



_OCTELEMETRY = _descriptor.ServiceDescriptor(
  name='OCTelemetry',
  full_name='Telemetry.OCTelemetry',
  file=DESCRIPTOR,
  index=0,
  serialized_options=None,
  serialized_start=891,
  serialized_end=1075,
  methods=[
  _descriptor.MethodDescriptor(
    name='telemetrySubscribe',
    full_name='Telemetry.OCTelemetry.telemetrySubscribe',
    index=0,
    containing_service=None,
    input_type=_SUBSCRIPTIONREQUEST,
    output_type=_SUBSCRIPTIONRESPONSE,
    serialized_options=None,
  ),
  _descriptor.MethodDescriptor(
    name='cancelTelemetrySubscription',
    full_name='Telemetry.OCTelemetry.cancelTelemetrySubscription',
    index=1,
    containing_service=None,
    input_type=_SUBSCRIPTIONID,
    output_type=_NOMESSAGE,
    serialized_options=None,
  ),
])
_sym_db.RegisterServiceDescriptor(_OCTELEMETRY)

DESCRIPTOR.services_by_name['OCTelemetry'] = _OCTELEMETRY


_OCREPLY = _descriptor.ServiceDescriptor(
  name='OCReply',
  full_name='Telemetry.OCReply',
  file=DESCRIPTOR,
  index=1,
  serialized_options=None,
  serialized_start=1077,
  serialized_end=1152,
  methods=[
  _descriptor.MethodDescriptor(
    name='StreamData',
    full_name='Telemetry.OCReply.StreamData',
    index=0,
    containing_service=None,
    input_type=_TELEMETRYDATA,
    output_type=_NOMESSAGE,
    serialized_options=None,
  ),
])
_sym_db.RegisterServiceDescriptor(_OCREPLY)

DESCRIPTOR.services_by_name['OCReply'] = _OCREPLY

# @@protoc_insertion_point(module_scope)
