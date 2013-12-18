# $Id$

srcdir=	../dist/..
builddir=.

##################################################
# Installation directories and permissions.
##################################################
prefix=	/usr
exec_prefix=${prefix}
bindir=	${exec_prefix}/bin
includedir=${prefix}/include
libdir=	${exec_prefix}/lib
docdir=	$(prefix)/docs

dmode=	755
emode=	555
fmode=	444

transform=s,x,x,

##################################################
# Paths for standard user-level commands.
##################################################
AR=	mipsel-uclibc-ar
CHMOD=	chmod
CP=	cp
LN=	ln
MKDIR=	mkdir
RANLIB=	mipsel-uclibc-ranlib
RM=	rm -f
SHELL=	/bin/sh
STRIP=	mipsel-uclibc-strip

##################################################
# General library information.
##################################################
DEF_LIB=	$(libso_target)
DEF_LIB_CXX=	
DEF_LIB_STL=	
INSTALLER=	$(LIBTOOL) --mode=install cp -p
LIBTOOL=	./libtool

POSTLINK=	$(LIBTOOL) --mode=execute true
SOLINK=		$(LIBTOOL) --mode=link mipsel-uclibc-gcc -avoid-version -Os -Wall -DLINUX26 -DCONFIG_BCMWL5 -pipe -DBCMWPA2 -funit-at-a-time -Wno-pointer-sign -mtune=mips32 -mips32   -fno-delete-null-pointer-checks -funit-at-a-time 	--param large-function-growth=800 --param max-inline-insns-single=3000 	-ffunction-sections -fdata-sections 
SOFLAGS=	-rpath $(libdir)
LIBMAJOR=	4
LIBVERSION=	4.8

CPPFLAGS=	-I$(builddir) -I$(srcdir)  -D_GNU_SOURCE -D_REENTRANT

##################################################
# C API.
##################################################
CFLAGS=		-c $(CPPFLAGS) -Os -Wall -DLINUX26 -DCONFIG_BCMWL5 -pipe -DBCMWPA2 -funit-at-a-time -Wno-pointer-sign -mtune=mips32 -mips32   -fno-delete-null-pointer-checks -funit-at-a-time 	--param large-function-growth=800 --param max-inline-insns-single=3000 	-ffunction-sections -fdata-sections 
CC=		$(LIBTOOL) --mode=compile mipsel-uclibc-gcc
CCLINK=		$(LIBTOOL) --mode=link mipsel-uclibc-gcc -Os -Wall -DLINUX26 -DCONFIG_BCMWL5 -pipe -DBCMWPA2 -funit-at-a-time -Wno-pointer-sign -mtune=mips32 -mips32   -fno-delete-null-pointer-checks -funit-at-a-time 	--param large-function-growth=800 --param max-inline-insns-single=3000 	-ffunction-sections -fdata-sections 

LDFLAGS=	
LIBS=		 -lpthread
TEST_LIBS=	 -lm -lnsl
LIBCSO_LIBS=	  -lpthread

libdb_base=	libdb
libdb=		$(libdb_base).a
libdb_version=	$(libdb_base)-$(LIBVERSION).a
libso=		$(libdb_base)-$(LIBVERSION).so
libso_target=	$(libdb_base)-$(LIBVERSION).la
libso_default=	$(libdb_base).so
libso_major=	$(libdb_base)-$(LIBMAJOR).so

##################################################
# C++ API.
#
# C++ support is optional, and can be built with static or shared libraries.
##################################################
CXXFLAGS=	-c $(CPPFLAGS) -Os -Wall -DLINUX26 -DCONFIG_BCMWL5 -pipe -DBCMWPA2 -funit-at-a-time -Wno-pointer-sign -mtune=mips32 -mips32   -fno-delete-null-pointer-checks -funit-at-a-time 	--param large-function-growth=800 --param max-inline-insns-single=3000 	-ffunction-sections -fdata-sections
CXX=		$(LIBTOOL) --mode=compile nocxx
CXXLINK=	$(LIBTOOL) --mode=link nocxx -Os -Wall -DLINUX26 -DCONFIG_BCMWL5 -pipe -DBCMWPA2 -funit-at-a-time -Wno-pointer-sign -mtune=mips32 -mips32   -fno-delete-null-pointer-checks -funit-at-a-time 	--param large-function-growth=800 --param max-inline-insns-single=3000 	-ffunction-sections -fdata-sections
XSOLINK=	$(LIBTOOL) --mode=link nocxx -avoid-version -Os -Wall -DLINUX26 -DCONFIG_BCMWL5 -pipe -DBCMWPA2 -funit-at-a-time -Wno-pointer-sign -mtune=mips32 -mips32   -fno-delete-null-pointer-checks -funit-at-a-time 	--param large-function-growth=800 --param max-inline-insns-single=3000 	-ffunction-sections -fdata-sections
LIBXSO_LIBS=	  -lpthread

libcxx_base=	libdb_cxx
libcxx=		$(libcxx_base).a
libcxx_version=	$(libcxx_base)-$(LIBVERSION).a
libxso=		$(libcxx_base)-$(LIBVERSION).so
libxso_target=	$(libcxx_base)-$(LIBVERSION).la
libxso_default=	$(libcxx_base).so
libxso_major=	$(libcxx_base)-$(LIBMAJOR).so

##################################################
# STL API.
#
# STL support is optional, and can be built with static or shared libraries.
##################################################
STLFLAGS=	$(CXXFLAGS) -I$(srcdir)/stl
LIBSTLSO_LIBS=	  -lpthread

libstl_base=	libdb_stl
libstl=		$(libstl_base).a
libstl_version=	$(libstl_base)-$(LIBVERSION).a
libstlso=	$(libstl_base)-$(LIBVERSION).so
libstlso_target=$(libstl_base)-$(LIBVERSION).la
libstlso_default=$(libstl_base).so
libstlso_major=	$(libstl_base)-$(LIBMAJOR).so

##################################################
# Java API.
#
# Java support is optional and requires shared librarires.
##################################################
CLASSPATH=	$(JAVA_CLASSTOP)
LIBJSO_LIBS=	  -lpthread
SWIGCFLAGS=	

JAR=		
JAVA=		env CLASSPATH="$(CLASSPATH)" 
JAVAC=		env CLASSPATH="$(CLASSPATH)" nojavac
JAVACFLAGS=	
JAVA_CLASSTOP=	./classes
JAVA_EXCLASSTOP=./classes.ex
JAVA_SRCDIR=	$(srcdir)/java/src
JAVA_EXDIR=	$(srcdir)/examples_java/src
JAVA_SLEEPYCAT=	$(srcdir)/java/src/com/sleepycat
JAVA_MANIFEST=	$(srcdir)/java/jarManifestEntries

libj_jarfile=	db.jar
libj_exjarfile=	dbexamples.jar
libjso_base=	libdb_java
libjso=		$(libjso_base)-$(LIBVERSION).so
libjso_static=	$(libjso_base)-$(LIBVERSION).a
libjso_target=	$(libjso_base)-$(LIBVERSION).la
libjso_default=	$(libjso_base).so
libjso_major=	$(libjso_base)-$(LIBMAJOR).so
libjso_g=	$(libjso_base)-$(LIBVERSION)_g.so

##################################################
# TCL API.
#
# Tcl support is optional and requires shared libraries.
##################################################
TCL_INCLUDE_SPEC=	
LIBTSO_LIBS=	  -lpthread
libtso_base=	libdb_tcl
libtso=		$(libtso_base)-$(LIBVERSION).so
libtso_static=	$(libtso_base)-$(LIBVERSION).a
libtso_target=	$(libtso_base)-$(LIBVERSION).la
libtso_default=	$(libtso_base).so
libtso_major=	$(libtso_base)-$(LIBMAJOR).so

##################################################
# db_dump185 UTILITY
#
# The db_dump185 application should be compiled using the system's db.h file
# (which should be a DB 1.85/1.86 include file), and the system's 1.85/1.86
# object library.  To include the right db.h, don't include -I$(builddir) on
# the compile line.  You may also need to add a local include directory and
# local libraries, for example.  Do that by adding -I options to the DB185INC
# line, and -l options to the DB185LIB line.
##################################################
DB185INC=	-c -Os -Wall -DLINUX26 -DCONFIG_BCMWL5 -pipe -DBCMWPA2 -funit-at-a-time -Wno-pointer-sign -mtune=mips32 -mips32   -fno-delete-null-pointer-checks -funit-at-a-time 	--param large-function-growth=800 --param max-inline-insns-single=3000 	-ffunction-sections -fdata-sections  -I$(srcdir)  -D_GNU_SOURCE -D_REENTRANT
DB185LIB=

##################################################
# NOTHING BELOW THIS LINE SHOULD EVER NEED TO BE MODIFIED.
##################################################

##################################################
# Object and utility lists.
##################################################
BTREE_OBJS=\
	bt_compare.lo bt_compress.lo bt_conv.lo bt_curadj.lo bt_cursor.lo \
	bt_delete.lo bt_method.lo bt_open.lo bt_put.lo bt_rec.lo \
	bt_reclaim.lo bt_recno.lo bt_rsearch.lo bt_search.lo bt_split.lo bt_stat.lo \
	bt_compact.lo bt_upgrade.lo btree_auto.lo
BTREE_VRFY_OBJS=\
	db_ovfl_vrfy.lo db_vrfy.lo db_vrfyutil.lo bt_verify.lo
HASH_OBJS=\
	hash.lo hash_auto.lo hash_conv.lo hash_dup.lo hash_meta.lo \
	hash_method.lo hash_open.lo hash_page.lo hash_rec.lo \
	hash_reclaim.lo hash_stat.lo hash_upgrade.lo
HASH_VRFY_OBJS=\
	hash_verify.lo
QUEUE_OBJS=\
	qam.lo qam_auto.lo qam_conv.lo qam_files.lo qam_method.lo \
	qam_open.lo qam_rec.lo qam_stat.lo qam_upgrade.lo
QUEUE_VRFY_OBJS=\
	qam_verify.lo
LOCK_OBJS=\
	lock.lo lock_deadlock.lo lock_failchk.lo lock_id.lo lock_list.lo \
	lock_method.lo lock_region.lo lock_stat.lo lock_timer.lo lock_util.lo
MUTEX_OBJS=\
	mut_alloc.lo mut_failchk.lo mut_method.lo mut_region.lo mut_stat.lo
REP_OBJS=\
	rep_auto.lo rep_backup.lo rep_elect.lo rep_lease.lo rep_log.lo \
	rep_method.lo rep_record.lo rep_region.lo rep_stat.lo \
	rep_util.lo rep_verify.lo
REPMGR_OBJS=\
	os_addrinfo.lo\
	repmgr_auto.lo repmgr_elect.lo repmgr_method.lo repmgr_msg.lo \
	repmgr_net.lo repmgr_posix.lo repmgr_queue.lo repmgr_sel.lo \
	repmgr_stat.lo repmgr_util.lo
PRINT_OBJS=\
	btree_autop.lo crdel_autop.lo db_autop.lo dbreg_autop.lo \
	fileops_autop.lo hash_autop.lo qam_autop.lo txn_autop.lo

C_OBJS= db185.lo mut_pthread.lo  $(BTREE_OBJS) $(HASH_OBJS) $(HASH_VRFY_OBJS) $(QUEUE_OBJS) $(QUEUE_VRFY_OBJS) $(REP_OBJS) $(REPMGR_OBJS) $(BTREE_VRFY_OBJS) $(LOCK_OBJS) $(MUTEX_OBJS)  $(CRYPTO_OBJS)  \
	crdel_auto.lo crdel_rec.lo db.lo db_am.lo db_auto.lo \
	db_byteorder.lo db_cam.lo db_cds.lo db_compint.lo db_conv.lo db_dispatch.lo \
	db_dup.lo db_err.lo db_getlong.lo db_idspace.lo db_iface.lo \
	db_join.lo db_log2.lo db_meta.lo db_method.lo db_open.lo \
	db_overflow.lo db_pr.lo db_rec.lo db_reclaim.lo db_remove.lo \
	db_rename.lo db_ret.lo db_setid.lo db_setlsn.lo db_shash.lo \
	db_sort_multiple.lo db_stati.lo db_truncate.lo db_upg.lo \
	db_upg_opd.lo dbm.lo dbreg.lo dbreg_auto.lo dbreg_rec.lo dbreg_stat.lo \
	dbreg_util.lo dbt.lo env_alloc.lo env_config.lo env_failchk.lo \
	env_file.lo env_globals.lo env_method.lo env_name.lo env_open.lo \
	env_recover.lo env_region.lo env_register.lo env_sig.lo \
	env_stat.lo fileops_auto.lo fop_basic.lo fop_rec.lo fop_util.lo \
	hash_func.lo hmac.lo hsearch.lo log.lo log_archive.lo \
	log_compare.lo log_debug.lo log_get.lo log_method.lo log_put.lo \
	log_stat.lo mkpath.lo mp_alloc.lo mp_bh.lo mp_fget.lo \
	mp_fmethod.lo mp_fopen.lo mp_fput.lo mp_fset.lo mp_method.lo \
	mp_mvcc.lo mp_region.lo mp_register.lo mp_resize.lo mp_stat.lo \
	mp_sync.lo mp_trickle.lo openflags.lo os_abort.lo os_abs.lo \
	os_alloc.lo os_clock.lo os_cpu.lo os_ctime.lo os_config.lo \
	os_dir.lo os_errno.lo os_fid.lo os_flock.lo os_fsync.lo \
	os_getenv.lo os_handle.lo os_map.lo os_method.lo os_mkdir.lo \
	os_open.lo os_pid.lo os_rename.lo os_root.lo os_rpath.lo \
	os_rw.lo os_seek.lo os_stack.lo os_stat.lo os_tmpdir.lo \
	os_truncate.lo os_uid.lo os_unlink.lo os_yield.lo partition.lo \
	seq_stat.lo sequence.lo sha1.lo snprintf.lo txn.lo txn_auto.lo \
	txn_chkpt.lo txn_failchk.lo txn_method.lo txn_rec.lo txn_recover.lo \
	txn_region.lo txn_stat.lo txn_util.lo zerofill.lo

CXX_OBJS=\
	cxx_db.lo cxx_dbc.lo cxx_dbt.lo cxx_env.lo cxx_except.lo cxx_lock.lo \
	cxx_logc.lo cxx_mpool.lo cxx_multi.lo cxx_seq.lo cxx_txn.lo

CRYPTO_OBJS=\
	aes_method.lo crypto.lo mt19937db.lo rijndael-alg-fst.lo \
	rijndael-api-fst.lo

JAVA_OBJS=\
	db_java_wrap.lo

JAVA_DBSRCS=\
	$(JAVA_SLEEPYCAT)/asm/AnnotationVisitor.java \
	$(JAVA_SLEEPYCAT)/asm/AnnotationWriter.java \
	$(JAVA_SLEEPYCAT)/asm/Attribute.java \
	$(JAVA_SLEEPYCAT)/asm/ByteVector.java \
	$(JAVA_SLEEPYCAT)/asm/ClassAdapter.java \
	$(JAVA_SLEEPYCAT)/asm/ClassReader.java \
	$(JAVA_SLEEPYCAT)/asm/ClassVisitor.java \
	$(JAVA_SLEEPYCAT)/asm/ClassWriter.java \
	$(JAVA_SLEEPYCAT)/asm/Edge.java \
	$(JAVA_SLEEPYCAT)/asm/FieldVisitor.java \
	$(JAVA_SLEEPYCAT)/asm/FieldWriter.java \
	$(JAVA_SLEEPYCAT)/asm/Handler.java \
	$(JAVA_SLEEPYCAT)/asm/Item.java \
	$(JAVA_SLEEPYCAT)/asm/Label.java \
	$(JAVA_SLEEPYCAT)/asm/MethodVisitor.java \
	$(JAVA_SLEEPYCAT)/asm/MethodWriter.java \
	$(JAVA_SLEEPYCAT)/asm/Opcodes.java \
	$(JAVA_SLEEPYCAT)/asm/Type.java \
	$(JAVA_SLEEPYCAT)/bind/ByteArrayBinding.java \
	$(JAVA_SLEEPYCAT)/bind/EntityBinding.java \
	$(JAVA_SLEEPYCAT)/bind/EntryBinding.java \
	$(JAVA_SLEEPYCAT)/bind/RecordNumberBinding.java \
	$(JAVA_SLEEPYCAT)/bind/serial/ClassCatalog.java \
	$(JAVA_SLEEPYCAT)/bind/serial/SerialBase.java \
	$(JAVA_SLEEPYCAT)/bind/serial/SerialBinding.java \
	$(JAVA_SLEEPYCAT)/bind/serial/SerialInput.java \
	$(JAVA_SLEEPYCAT)/bind/serial/SerialOutput.java \
	$(JAVA_SLEEPYCAT)/bind/serial/SerialSerialBinding.java \
	$(JAVA_SLEEPYCAT)/bind/serial/SerialSerialKeyCreator.java \
	$(JAVA_SLEEPYCAT)/bind/serial/StoredClassCatalog.java \
	$(JAVA_SLEEPYCAT)/bind/serial/TupleSerialBinding.java \
	$(JAVA_SLEEPYCAT)/bind/serial/TupleSerialKeyCreator.java \
	$(JAVA_SLEEPYCAT)/bind/serial/TupleSerialMarshalledBinding.java \
	$(JAVA_SLEEPYCAT)/bind/serial/TupleSerialMarshalledKeyCreator.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/BigIntegerBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/BooleanBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/ByteBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/CharacterBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/DoubleBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/FloatBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/IntegerBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/LongBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/MarshalledTupleEntry.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/MarshalledTupleKeyEntity.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/ShortBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/SortedDoubleBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/SortedFloatBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/StringBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/TupleBase.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/TupleBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/TupleInput.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/TupleInputBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/TupleMarshalledBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/TupleOutput.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/TupleTupleBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/TupleTupleKeyCreator.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/TupleTupleMarshalledBinding.java \
	$(JAVA_SLEEPYCAT)/bind/tuple/TupleTupleMarshalledKeyCreator.java \
	$(JAVA_SLEEPYCAT)/collections/BaseIterator.java \
	$(JAVA_SLEEPYCAT)/collections/BlockIterator.java \
	$(JAVA_SLEEPYCAT)/collections/CurrentTransaction.java \
	$(JAVA_SLEEPYCAT)/collections/DataCursor.java \
	$(JAVA_SLEEPYCAT)/collections/DataView.java \
	$(JAVA_SLEEPYCAT)/collections/MapEntryParameter.java \
	$(JAVA_SLEEPYCAT)/collections/MyRangeCursor.java \
	$(JAVA_SLEEPYCAT)/collections/PrimaryKeyAssigner.java \
	$(JAVA_SLEEPYCAT)/collections/StoredCollection.java \
	$(JAVA_SLEEPYCAT)/collections/StoredCollections.java \
	$(JAVA_SLEEPYCAT)/collections/StoredContainer.java \
	$(JAVA_SLEEPYCAT)/collections/StoredEntrySet.java \
	$(JAVA_SLEEPYCAT)/collections/StoredIterator.java \
	$(JAVA_SLEEPYCAT)/collections/StoredKeySet.java \
	$(JAVA_SLEEPYCAT)/collections/StoredList.java \
	$(JAVA_SLEEPYCAT)/collections/StoredMap.java \
	$(JAVA_SLEEPYCAT)/collections/StoredMapEntry.java \
	$(JAVA_SLEEPYCAT)/collections/StoredSortedEntrySet.java \
	$(JAVA_SLEEPYCAT)/collections/StoredSortedKeySet.java \
	$(JAVA_SLEEPYCAT)/collections/StoredSortedMap.java \
	$(JAVA_SLEEPYCAT)/collections/StoredSortedValueSet.java \
	$(JAVA_SLEEPYCAT)/collections/StoredValueSet.java \
	$(JAVA_SLEEPYCAT)/collections/TransactionRunner.java \
	$(JAVA_SLEEPYCAT)/collections/TransactionWorker.java \
	$(JAVA_SLEEPYCAT)/collections/TupleSerialFactory.java \
	$(JAVA_SLEEPYCAT)/compat/DbCompat.java \
	$(JAVA_SLEEPYCAT)/db/BtreeCompressor.java \
	$(JAVA_SLEEPYCAT)/db/BtreePrefixCalculator.java \
	$(JAVA_SLEEPYCAT)/db/BtreeStats.java \
	$(JAVA_SLEEPYCAT)/db/CacheFile.java \
	$(JAVA_SLEEPYCAT)/db/CacheFilePriority.java \
	$(JAVA_SLEEPYCAT)/db/CacheFileStats.java \
	$(JAVA_SLEEPYCAT)/db/CacheStats.java \
	$(JAVA_SLEEPYCAT)/db/CheckpointConfig.java \
	$(JAVA_SLEEPYCAT)/db/CompactConfig.java \
	$(JAVA_SLEEPYCAT)/db/CompactStats.java \
	$(JAVA_SLEEPYCAT)/db/Cursor.java \
	$(JAVA_SLEEPYCAT)/db/CursorConfig.java \
	$(JAVA_SLEEPYCAT)/db/Database.java \
	$(JAVA_SLEEPYCAT)/db/DatabaseConfig.java \
	$(JAVA_SLEEPYCAT)/db/DatabaseEntry.java \
	$(JAVA_SLEEPYCAT)/db/DatabaseException.java \
	$(JAVA_SLEEPYCAT)/db/DatabaseStats.java \
	$(JAVA_SLEEPYCAT)/db/DatabaseType.java \
	$(JAVA_SLEEPYCAT)/db/DeadlockException.java \
	$(JAVA_SLEEPYCAT)/db/Environment.java \
	$(JAVA_SLEEPYCAT)/db/EnvironmentConfig.java \
	$(JAVA_SLEEPYCAT)/db/ErrorHandler.java \
	$(JAVA_SLEEPYCAT)/db/EventHandler.java \
	$(JAVA_SLEEPYCAT)/db/EventHandlerAdapter.java \
	$(JAVA_SLEEPYCAT)/db/FeedbackHandler.java \
	$(JAVA_SLEEPYCAT)/db/ForeignKeyDeleteAction.java \
	$(JAVA_SLEEPYCAT)/db/ForeignKeyNullifier.java \
	$(JAVA_SLEEPYCAT)/db/ForeignMultiKeyNullifier.java \
	$(JAVA_SLEEPYCAT)/db/HashStats.java \
	$(JAVA_SLEEPYCAT)/db/Hasher.java \
	$(JAVA_SLEEPYCAT)/db/JoinConfig.java \
	$(JAVA_SLEEPYCAT)/db/JoinCursor.java \
	$(JAVA_SLEEPYCAT)/db/KeyRange.java \
	$(JAVA_SLEEPYCAT)/db/Lock.java \
	$(JAVA_SLEEPYCAT)/db/LockDetectMode.java \
	$(JAVA_SLEEPYCAT)/db/LockMode.java \
	$(JAVA_SLEEPYCAT)/db/LockNotGrantedException.java \
	$(JAVA_SLEEPYCAT)/db/LockOperation.java \
	$(JAVA_SLEEPYCAT)/db/LockRequest.java \
	$(JAVA_SLEEPYCAT)/db/LockRequestMode.java \
	$(JAVA_SLEEPYCAT)/db/LockStats.java \
	$(JAVA_SLEEPYCAT)/db/LogCursor.java \
	$(JAVA_SLEEPYCAT)/db/LogRecordHandler.java \
	$(JAVA_SLEEPYCAT)/db/LogSequenceNumber.java \
	$(JAVA_SLEEPYCAT)/db/LogStats.java \
	$(JAVA_SLEEPYCAT)/db/MemoryException.java \
	$(JAVA_SLEEPYCAT)/db/MessageHandler.java \
	$(JAVA_SLEEPYCAT)/db/MultipleNIODataEntry.java \
	$(JAVA_SLEEPYCAT)/db/MultipleKeyNIODataEntry.java \
	$(JAVA_SLEEPYCAT)/db/MultipleRecnoNIODataEntry.java \
	$(JAVA_SLEEPYCAT)/db/MultipleDataEntry.java \
	$(JAVA_SLEEPYCAT)/db/MultipleEntry.java \
	$(JAVA_SLEEPYCAT)/db/MultipleKeyDataEntry.java \
	$(JAVA_SLEEPYCAT)/db/MultipleRecnoDataEntry.java \
	$(JAVA_SLEEPYCAT)/db/MutexStats.java \
	$(JAVA_SLEEPYCAT)/db/OperationStatus.java \
	$(JAVA_SLEEPYCAT)/db/PanicHandler.java \
	$(JAVA_SLEEPYCAT)/db/PartitionHandler.java \
	$(JAVA_SLEEPYCAT)/db/PreparedTransaction.java \
	$(JAVA_SLEEPYCAT)/db/QueueStats.java \
	$(JAVA_SLEEPYCAT)/db/RecordNumberAppender.java \
	$(JAVA_SLEEPYCAT)/db/RecoveryOperation.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationConfig.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationDuplicateMasterException.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationHandleDeadException.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationHoldElectionException.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationHostAddress.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationJoinFailureException.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationLeaseExpiredException.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationLeaseTimeoutException.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationLockoutException.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationManagerAckPolicy.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationManagerSiteInfo.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationManagerStats.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationManagerStartPolicy.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationSiteUnavailableException.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationStats.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationStatus.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationTimeoutType.java \
	$(JAVA_SLEEPYCAT)/db/ReplicationTransport.java \
	$(JAVA_SLEEPYCAT)/db/RunRecoveryException.java \
	$(JAVA_SLEEPYCAT)/db/SecondaryConfig.java \
	$(JAVA_SLEEPYCAT)/db/SecondaryCursor.java \
	$(JAVA_SLEEPYCAT)/db/SecondaryDatabase.java \
	$(JAVA_SLEEPYCAT)/db/SecondaryKeyCreator.java \
	$(JAVA_SLEEPYCAT)/db/SecondaryMultiKeyCreator.java \
	$(JAVA_SLEEPYCAT)/db/Sequence.java \
	$(JAVA_SLEEPYCAT)/db/SequenceConfig.java \
	$(JAVA_SLEEPYCAT)/db/SequenceStats.java \
	$(JAVA_SLEEPYCAT)/db/StatsConfig.java \
	$(JAVA_SLEEPYCAT)/db/Transaction.java \
	$(JAVA_SLEEPYCAT)/db/TransactionConfig.java \
	$(JAVA_SLEEPYCAT)/db/TransactionStats.java \
	$(JAVA_SLEEPYCAT)/db/VerboseConfig.java \
	$(JAVA_SLEEPYCAT)/db/VerifyConfig.java \
	$(JAVA_SLEEPYCAT)/db/VersionMismatchException.java \
	$(JAVA_SLEEPYCAT)/db/internal/Db.java \
	$(JAVA_SLEEPYCAT)/db/internal/DbConstants.java \
	$(JAVA_SLEEPYCAT)/db/internal/DbEnv.java \
	$(JAVA_SLEEPYCAT)/db/internal/DbLock.java \
	$(JAVA_SLEEPYCAT)/db/internal/DbLogc.java \
	$(JAVA_SLEEPYCAT)/db/internal/DbMpoolFile.java \
	$(JAVA_SLEEPYCAT)/db/internal/DbSequence.java \
	$(JAVA_SLEEPYCAT)/db/internal/DbTxn.java \
	$(JAVA_SLEEPYCAT)/db/internal/DbUtil.java \
	$(JAVA_SLEEPYCAT)/db/internal/Dbc.java \
	$(JAVA_SLEEPYCAT)/db/internal/db_java.java \
	$(JAVA_SLEEPYCAT)/db/internal/db_javaJNI.java \
	$(JAVA_SLEEPYCAT)/persist/BasicCursor.java \
	$(JAVA_SLEEPYCAT)/persist/BasicIndex.java \
	$(JAVA_SLEEPYCAT)/persist/BasicIterator.java \
	$(JAVA_SLEEPYCAT)/persist/DataValueAdapter.java \
	$(JAVA_SLEEPYCAT)/persist/DatabaseNamer.java \
	$(JAVA_SLEEPYCAT)/persist/EntityCursor.java \
	$(JAVA_SLEEPYCAT)/persist/EntityIndex.java \
	$(JAVA_SLEEPYCAT)/persist/EntityJoin.java \
	$(JAVA_SLEEPYCAT)/persist/EntityStore.java \
	$(JAVA_SLEEPYCAT)/persist/EntityValueAdapter.java \
	$(JAVA_SLEEPYCAT)/persist/ForwardCursor.java \
	$(JAVA_SLEEPYCAT)/persist/KeySelector.java \
	$(JAVA_SLEEPYCAT)/persist/KeyValueAdapter.java \
	$(JAVA_SLEEPYCAT)/persist/KeysIndex.java \
	$(JAVA_SLEEPYCAT)/persist/PrimaryIndex.java \
	$(JAVA_SLEEPYCAT)/persist/PrimaryKeyValueAdapter.java \
	$(JAVA_SLEEPYCAT)/persist/SecondaryIndex.java \
	$(JAVA_SLEEPYCAT)/persist/StoreConfig.java \
	$(JAVA_SLEEPYCAT)/persist/StoreExistsException.java \
	$(JAVA_SLEEPYCAT)/persist/StoreNotFoundException.java \
	$(JAVA_SLEEPYCAT)/persist/SubIndex.java \
	$(JAVA_SLEEPYCAT)/persist/SubIndexCursor.java \
	$(JAVA_SLEEPYCAT)/persist/ValueAdapter.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/Conversion.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/Converter.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/DeletedClassException.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/Deleter.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/EntityConverter.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/EvolveConfig.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/EvolveEvent.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/EvolveInternal.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/EvolveListener.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/EvolveStats.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/IncompatibleClassException.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/Mutation.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/Mutations.java \
	$(JAVA_SLEEPYCAT)/persist/evolve/Renamer.java \
	$(JAVA_SLEEPYCAT)/persist/impl/AbstractInput.java \
	$(JAVA_SLEEPYCAT)/persist/impl/Accessor.java \
	$(JAVA_SLEEPYCAT)/persist/impl/Catalog.java \
	$(JAVA_SLEEPYCAT)/persist/impl/CollectionProxy.java \
	$(JAVA_SLEEPYCAT)/persist/impl/ComplexFormat.java \
	$(JAVA_SLEEPYCAT)/persist/impl/ComparatorCatalog.java \
	$(JAVA_SLEEPYCAT)/persist/impl/CompositeKeyFormat.java \
	$(JAVA_SLEEPYCAT)/persist/impl/ConverterReader.java \
	$(JAVA_SLEEPYCAT)/persist/impl/Enhanced.java \
	$(JAVA_SLEEPYCAT)/persist/impl/EnhancedAccessor.java \
	$(JAVA_SLEEPYCAT)/persist/impl/EntityInput.java \
	$(JAVA_SLEEPYCAT)/persist/impl/EntityOutput.java \
	$(JAVA_SLEEPYCAT)/persist/impl/EnumFormat.java \
	$(JAVA_SLEEPYCAT)/persist/impl/Evolver.java \
	$(JAVA_SLEEPYCAT)/persist/impl/FieldInfo.java \
	$(JAVA_SLEEPYCAT)/persist/impl/Format.java \
	$(JAVA_SLEEPYCAT)/persist/impl/KeyLocation.java \
	$(JAVA_SLEEPYCAT)/persist/impl/MapProxy.java \
	$(JAVA_SLEEPYCAT)/persist/impl/NonPersistentFormat.java \
	$(JAVA_SLEEPYCAT)/persist/impl/ObjectArrayFormat.java \
	$(JAVA_SLEEPYCAT)/persist/impl/PersistCatalog.java \
	$(JAVA_SLEEPYCAT)/persist/impl/PersistComparator.java \
	$(JAVA_SLEEPYCAT)/persist/impl/PersistEntityBinding.java \
	$(JAVA_SLEEPYCAT)/persist/impl/PersistKeyAssigner.java \
	$(JAVA_SLEEPYCAT)/persist/impl/PersistKeyBinding.java \
	$(JAVA_SLEEPYCAT)/persist/impl/PersistKeyCreator.java \
	$(JAVA_SLEEPYCAT)/persist/impl/PrimitiveArrayFormat.java \
	$(JAVA_SLEEPYCAT)/persist/impl/ProxiedFormat.java \
	$(JAVA_SLEEPYCAT)/persist/impl/RawAbstractInput.java \
	$(JAVA_SLEEPYCAT)/persist/impl/RawAccessor.java \
	$(JAVA_SLEEPYCAT)/persist/impl/RawArrayInput.java \
	$(JAVA_SLEEPYCAT)/persist/impl/RawComplexInput.java \
	$(JAVA_SLEEPYCAT)/persist/impl/RawSingleInput.java \
	$(JAVA_SLEEPYCAT)/persist/impl/ReadOnlyCatalog.java \
	$(JAVA_SLEEPYCAT)/persist/impl/Reader.java \
	$(JAVA_SLEEPYCAT)/persist/impl/RecordInput.java \
	$(JAVA_SLEEPYCAT)/persist/impl/RecordOutput.java \
	$(JAVA_SLEEPYCAT)/persist/impl/ReflectionAccessor.java \
	$(JAVA_SLEEPYCAT)/persist/impl/SimpleCatalog.java \
	$(JAVA_SLEEPYCAT)/persist/impl/SimpleFormat.java \
	$(JAVA_SLEEPYCAT)/persist/impl/Store.java \
	$(JAVA_SLEEPYCAT)/persist/impl/StoredModel.java \
	$(JAVA_SLEEPYCAT)/persist/impl/VisitedObjects.java \
	$(JAVA_SLEEPYCAT)/persist/impl/WidenerInput.java \
	$(JAVA_SLEEPYCAT)/persist/model/AnnotationModel.java \
	$(JAVA_SLEEPYCAT)/persist/model/BytecodeEnhancer.java \
	$(JAVA_SLEEPYCAT)/persist/model/ClassEnhancer.java \
	$(JAVA_SLEEPYCAT)/persist/model/ClassMetadata.java \
	$(JAVA_SLEEPYCAT)/persist/model/DeleteAction.java \
	$(JAVA_SLEEPYCAT)/persist/model/Entity.java \
	$(JAVA_SLEEPYCAT)/persist/model/EntityMetadata.java \
	$(JAVA_SLEEPYCAT)/persist/model/EntityModel.java \
	$(JAVA_SLEEPYCAT)/persist/model/FieldMetadata.java \
	$(JAVA_SLEEPYCAT)/persist/model/KeyField.java \
	$(JAVA_SLEEPYCAT)/persist/model/ModelInternal.java \
	$(JAVA_SLEEPYCAT)/persist/model/NotPersistent.java \
	$(JAVA_SLEEPYCAT)/persist/model/NotTransient.java \
	$(JAVA_SLEEPYCAT)/persist/model/Persistent.java \
	$(JAVA_SLEEPYCAT)/persist/model/PersistentProxy.java \
	$(JAVA_SLEEPYCAT)/persist/model/PrimaryKey.java \
	$(JAVA_SLEEPYCAT)/persist/model/PrimaryKeyMetadata.java \
	$(JAVA_SLEEPYCAT)/persist/model/Relationship.java \
	$(JAVA_SLEEPYCAT)/persist/model/SecondaryKey.java \
	$(JAVA_SLEEPYCAT)/persist/model/SecondaryKeyMetadata.java \
	$(JAVA_SLEEPYCAT)/persist/raw/RawField.java \
	$(JAVA_SLEEPYCAT)/persist/raw/RawObject.java \
	$(JAVA_SLEEPYCAT)/persist/raw/RawStore.java \
	$(JAVA_SLEEPYCAT)/persist/raw/RawType.java \
	$(JAVA_SLEEPYCAT)/util/ErrorBuffer.java \
	$(JAVA_SLEEPYCAT)/util/ExceptionUnwrapper.java \
	$(JAVA_SLEEPYCAT)/util/ExceptionWrapper.java \
	$(JAVA_SLEEPYCAT)/util/FastInputStream.java \
	$(JAVA_SLEEPYCAT)/util/FastOutputStream.java \
	$(JAVA_SLEEPYCAT)/util/IOExceptionWrapper.java \
	$(JAVA_SLEEPYCAT)/util/PackedInteger.java \
	$(JAVA_SLEEPYCAT)/util/RuntimeExceptionWrapper.java \
	$(JAVA_SLEEPYCAT)/util/UtfOps.java \
	$(JAVA_SLEEPYCAT)/util/keyrange/KeyRange.java \
	$(JAVA_SLEEPYCAT)/util/keyrange/KeyRangeException.java \
	$(JAVA_SLEEPYCAT)/util/keyrange/RangeCursor.java

JAVA_EXSRCS=\
	$(JAVA_EXDIR)/collections/access/AccessExample.java \
	$(JAVA_EXDIR)/collections/hello/HelloDatabaseWorld.java \
	$(JAVA_EXDIR)/collections/ship/basic/PartData.java \
	$(JAVA_EXDIR)/collections/ship/basic/PartKey.java \
	$(JAVA_EXDIR)/collections/ship/basic/Sample.java \
	$(JAVA_EXDIR)/collections/ship/basic/SampleDatabase.java \
	$(JAVA_EXDIR)/collections/ship/basic/SampleViews.java \
	$(JAVA_EXDIR)/collections/ship/basic/ShipmentData.java \
	$(JAVA_EXDIR)/collections/ship/basic/ShipmentKey.java \
	$(JAVA_EXDIR)/collections/ship/basic/SupplierData.java \
	$(JAVA_EXDIR)/collections/ship/basic/SupplierKey.java \
	$(JAVA_EXDIR)/collections/ship/basic/Weight.java \
	$(JAVA_EXDIR)/collections/ship/entity/Part.java \
	$(JAVA_EXDIR)/collections/ship/entity/PartData.java \
	$(JAVA_EXDIR)/collections/ship/entity/PartKey.java \
	$(JAVA_EXDIR)/collections/ship/entity/Sample.java \
	$(JAVA_EXDIR)/collections/ship/entity/SampleDatabase.java \
	$(JAVA_EXDIR)/collections/ship/entity/SampleViews.java \
	$(JAVA_EXDIR)/collections/ship/entity/Shipment.java \
	$(JAVA_EXDIR)/collections/ship/entity/ShipmentData.java \
	$(JAVA_EXDIR)/collections/ship/entity/ShipmentKey.java \
	$(JAVA_EXDIR)/collections/ship/entity/Supplier.java \
	$(JAVA_EXDIR)/collections/ship/entity/SupplierData.java \
	$(JAVA_EXDIR)/collections/ship/entity/SupplierKey.java \
	$(JAVA_EXDIR)/collections/ship/entity/Weight.java \
	$(JAVA_EXDIR)/collections/ship/factory/Part.java \
	$(JAVA_EXDIR)/collections/ship/factory/PartKey.java \
	$(JAVA_EXDIR)/collections/ship/factory/Sample.java \
	$(JAVA_EXDIR)/collections/ship/factory/SampleDatabase.java \
	$(JAVA_EXDIR)/collections/ship/factory/SampleViews.java \
	$(JAVA_EXDIR)/collections/ship/factory/Shipment.java \
	$(JAVA_EXDIR)/collections/ship/factory/ShipmentKey.java \
	$(JAVA_EXDIR)/collections/ship/factory/Supplier.java \
	$(JAVA_EXDIR)/collections/ship/factory/SupplierKey.java \
	$(JAVA_EXDIR)/collections/ship/factory/Weight.java \
	$(JAVA_EXDIR)/collections/ship/index/PartData.java \
	$(JAVA_EXDIR)/collections/ship/index/PartKey.java \
	$(JAVA_EXDIR)/collections/ship/index/Sample.java \
	$(JAVA_EXDIR)/collections/ship/index/SampleDatabase.java \
	$(JAVA_EXDIR)/collections/ship/index/SampleViews.java \
	$(JAVA_EXDIR)/collections/ship/index/ShipmentData.java \
	$(JAVA_EXDIR)/collections/ship/index/ShipmentKey.java \
	$(JAVA_EXDIR)/collections/ship/index/SupplierData.java \
	$(JAVA_EXDIR)/collections/ship/index/SupplierKey.java \
	$(JAVA_EXDIR)/collections/ship/index/Weight.java \
	$(JAVA_EXDIR)/collections/ship/marshal/MarshalledEnt.java \
	$(JAVA_EXDIR)/collections/ship/marshal/MarshalledKey.java \
	$(JAVA_EXDIR)/collections/ship/marshal/Part.java \
	$(JAVA_EXDIR)/collections/ship/marshal/PartKey.java \
	$(JAVA_EXDIR)/collections/ship/marshal/Sample.java \
	$(JAVA_EXDIR)/collections/ship/marshal/SampleDatabase.java \
	$(JAVA_EXDIR)/collections/ship/marshal/SampleViews.java \
	$(JAVA_EXDIR)/collections/ship/marshal/Shipment.java \
	$(JAVA_EXDIR)/collections/ship/marshal/ShipmentKey.java \
	$(JAVA_EXDIR)/collections/ship/marshal/Supplier.java \
	$(JAVA_EXDIR)/collections/ship/marshal/SupplierKey.java \
	$(JAVA_EXDIR)/collections/ship/marshal/Weight.java \
	$(JAVA_EXDIR)/collections/ship/sentity/Part.java \
	$(JAVA_EXDIR)/collections/ship/sentity/PartKey.java \
	$(JAVA_EXDIR)/collections/ship/sentity/Sample.java \
	$(JAVA_EXDIR)/collections/ship/sentity/SampleDatabase.java \
	$(JAVA_EXDIR)/collections/ship/sentity/SampleViews.java \
	$(JAVA_EXDIR)/collections/ship/sentity/Shipment.java \
	$(JAVA_EXDIR)/collections/ship/sentity/ShipmentKey.java \
	$(JAVA_EXDIR)/collections/ship/sentity/Supplier.java \
	$(JAVA_EXDIR)/collections/ship/sentity/SupplierKey.java \
	$(JAVA_EXDIR)/collections/ship/sentity/Weight.java \
	$(JAVA_EXDIR)/collections/ship/tuple/Part.java \
	$(JAVA_EXDIR)/collections/ship/tuple/PartData.java \
	$(JAVA_EXDIR)/collections/ship/tuple/PartKey.java \
	$(JAVA_EXDIR)/collections/ship/tuple/Sample.java \
	$(JAVA_EXDIR)/collections/ship/tuple/SampleDatabase.java \
	$(JAVA_EXDIR)/collections/ship/tuple/SampleViews.java \
	$(JAVA_EXDIR)/collections/ship/tuple/Shipment.java \
	$(JAVA_EXDIR)/collections/ship/tuple/ShipmentData.java \
	$(JAVA_EXDIR)/collections/ship/tuple/ShipmentKey.java \
	$(JAVA_EXDIR)/collections/ship/tuple/Supplier.java \
	$(JAVA_EXDIR)/collections/ship/tuple/SupplierData.java \
	$(JAVA_EXDIR)/collections/ship/tuple/SupplierKey.java \
	$(JAVA_EXDIR)/collections/ship/tuple/Weight.java \
	$(JAVA_EXDIR)/db/AccessExample.java \
	$(JAVA_EXDIR)/db/BtRecExample.java \
	$(JAVA_EXDIR)/db/BulkAccessExample.java \
	$(JAVA_EXDIR)/db/BulkAccessNIOExample.java \
	$(JAVA_EXDIR)/db/EnvExample.java \
	$(JAVA_EXDIR)/db/GettingStarted/ExampleDatabaseLoad.java \
	$(JAVA_EXDIR)/db/GettingStarted/ExampleDatabaseRead.java \
	$(JAVA_EXDIR)/db/GettingStarted/Inventory.java \
	$(JAVA_EXDIR)/db/GettingStarted/InventoryBinding.java \
	$(JAVA_EXDIR)/db/GettingStarted/ItemNameKeyCreator.java \
	$(JAVA_EXDIR)/db/GettingStarted/MyDbs.java \
	$(JAVA_EXDIR)/db/GettingStarted/Vendor.java \
	$(JAVA_EXDIR)/db/LockExample.java \
	$(JAVA_EXDIR)/db/SequenceExample.java \
	$(JAVA_EXDIR)/db/TpcbExample.java \
	$(JAVA_EXDIR)/db/repquote/RepConfig.java \
	$(JAVA_EXDIR)/db/repquote/RepQuoteEnvironment.java \
	$(JAVA_EXDIR)/db/repquote/RepQuoteExample.java \
	$(JAVA_EXDIR)/db/repquote/RepRemoteHost.java \
	$(JAVA_EXDIR)/db/repquote_gsg/RepConfig.java \
	$(JAVA_EXDIR)/db/repquote_gsg/RepQuoteEnvironment.java \
	$(JAVA_EXDIR)/db/repquote_gsg/RepQuoteExampleGSG.java \
	$(JAVA_EXDIR)/db/repquote_gsg/SimpleConfig.java \
	$(JAVA_EXDIR)/db/repquote_gsg/SimpleTxn.java \
	$(JAVA_EXDIR)/db/txn/DBWriter.java \
	$(JAVA_EXDIR)/db/txn/PayloadData.java \
	$(JAVA_EXDIR)/db/txn/TxnGuide.java \
	$(JAVA_EXDIR)/db/txn/TxnGuideInMemory.java \
	$(JAVA_EXDIR)/persist/CustomKeyOrderExample.java \
	$(JAVA_EXDIR)/persist/DplDump.java \
	$(JAVA_EXDIR)/persist/EventExample.java \
	$(JAVA_EXDIR)/persist/EventExampleDPL.java \
	$(JAVA_EXDIR)/persist/PersonExample.java \
	$(JAVA_EXDIR)/persist/gettingStarted/SimpleDA.java \
	$(JAVA_EXDIR)/persist/gettingStarted/SimpleEntityClass.java \
	$(JAVA_EXDIR)/persist/gettingStarted/SimpleStoreGet.java \
	$(JAVA_EXDIR)/persist/gettingStarted/SimpleStorePut.java \
	$(JAVA_EXDIR)/persist/txn/PayloadDataEntity.java \
	$(JAVA_EXDIR)/persist/txn/StoreWriter.java \
	$(JAVA_EXDIR)/persist/txn/TxnGuideDPL.java 

STL_OBJS=\
	dbstl_container.lo dbstl_resource_manager.lo

TCL_OBJS=\
	tcl_compat.lo tcl_db.lo tcl_db_pkg.lo tcl_dbcursor.lo tcl_env.lo \
	tcl_internal.lo tcl_lock.lo tcl_log.lo tcl_mp.lo tcl_mutex.lo \
	tcl_rep.lo tcl_seq.lo tcl_txn.lo tcl_util.lo

RPC_CLIENT_OBJS=\
	client.lo db_server_clnt.lo db_server_xdr.lo gen_client.lo \
	gen_client_ret.lo

RPC_SRV_OBJS=\
	db_server_proc.lo db_server_svc.lo db_server_util.lo \
	gen_db_server.lo

RPC_CXXSRV_OBJS=\
	db_server_cxxproc.lo db_server_cxxutil.lo db_server_svc.lo \
	gen_db_server.lo

TEST_MICRO_OBJS=\
	b_curalloc.lo b_curwalk.lo b_del.lo b_get.lo b_inmem.lo b_latch.lo \
	b_load.lo b_open.lo b_put.lo b_recover.lo b_txn.lo b_txn_write.lo \
	b_uname.lo b_util.lo b_workload.lo test_micro.lo util_arg.lo

UTIL_PROGS=\
	 \
	db_archive db_checkpoint db_deadlock db_dump \
	db_hotbackup db_load db_printlog db_recover db_sql db_stat db_upgrade \
	db_verify

##################################################
# List of files installed into the library directory.
##################################################
LIB_INSTALL_FILE_LIST=\
	$(libdb) \
	$(libso) \
	$(libso_default) \
	$(libso_major) \
	$(libdb_version) \
	$(libso_target) \
	$(libcxx) \
	$(libxso) \
	$(libxso_default) \
	$(libxso_major) \
	$(libcxx_version) \
	$(libxso_target) \
	$(libstl) \
	$(libstlso) \
	$(libstlso_default) \
	$(libstlso_major) \
	$(libstlxx_version) \
	$(libstlso_target) \
	$(libtso) \
	$(libtso_default) \
	$(libtso_major) \
	$(libtso_static) \
	$(libtso_target) \
	$(libjso) \
	$(libjso_default) \
	$(libjso_g) \
	$(libjso_major) \
	$(libjso_static) \
	$(libjso_target) \
	$(libj_exjarfile) \
	$(libj_jarfile)

##################################################
# Note: "all" must be the first target in the Makefile.
##################################################
all: library_build

install: all library_install

##################################################
# Library and standard utilities build.
##################################################
library_build: $(libso_target)  $(UTIL_PROGS)

# Static C library named libdb.a.
$(libdb): $(DEF_LIB)

# Real static C library.
$(libdb_version): $(C_OBJS)
	$(AR) cr $@ $(C_OBJS)
	$(RANLIB) $@
	$(RM) $(libdb)
	$(LN) -s $(libdb_version) $(libdb)

# Shared C library.
$(libso_target): $(C_OBJS)
	$(SOLINK) $(SOFLAGS) $(LDFLAGS) -o $@ $(C_OBJS) $(LIBCSO_LIBS)
	$(RM) $(libdb)
	$(LN) -s .libs/$(libdb_version) $(libdb)

# Static C++ library named libdb_cxx.a.
$(libcxx): $(DEF_LIB_CXX)

# Real static C++ library.
$(libcxx_version): $(CXX_OBJS) $(C_OBJS)
	$(AR) cr $@ $(CXX_OBJS) $(C_OBJS)
	$(RANLIB) $@
	$(RM) $(libcxx)
	$(LN) -s $(libcxx_version) $(libcxx)

# Shared C++ library.
$(libxso_target): $(CXX_OBJS) $(C_OBJS)
	$(XSOLINK) $(SOFLAGS) $(LDFLAGS) \
	    -o $@ $(CXX_OBJS) $(C_OBJS) $(LIBXSO_LIBS)
	$(RM) $(libcxx)
	$(LN) -s .libs/$(libcxx_version) $(libcxx)

# Static STL library named libdb_stl.a.
$(libstl): $(DEF_LIB_STL)

# Real static STL library.
$(libstl_version): $(STL_OBJS) $(CXX_OBJS) $(C_OBJS)
	$(AR) cr $@ $(STL_OBJS) $(CXX_OBJS) $(C_OBJS)
	$(RANLIB) $@
	$(RM) $(libstl)
	$(LN) -s $(libstl_version) $(libstl)

# Shared STL library.
$(libstlso_target): $(STL_OBJS) $(CXX_OBJS) $(C_OBJS)
	$(XSOLINK) $(SOFLAGS) $(LDFLAGS) -o $@ \
	    $(STL_OBJS) $(LIBSTLSO_LIBS) $(CXX_OBJS) $(C_OBJS) $(LIBXSO_LIBS)
	$(RM) $(libstl)
	$(LN) -s .libs/$(libstl_version) $(libstl)

# Shared Java library.
$(libjso_target): $(JAVA_OBJS) $(C_OBJS)
	$(SOLINK) -shrext .so $(SOFLAGS) $(LDFLAGS) \
	    -o $@ $(JAVA_OBJS) $(C_OBJS) $(LIBJSO_LIBS)

# Shared Tcl library.
$(libtso_target): $(TCL_OBJS) $(C_OBJS)
	$(SOLINK) -module $(SOFLAGS) $(LDFLAGS) \
	    -o $@ $(TCL_OBJS) $(C_OBJS) $(LIBTSO_LIBS)

##################################################
# Creating individual dependencies and actions for building class
# files is possible, but it is very messy and error prone.
##################################################
java:	$(libj_jarfile) $(libj_exjarfile)

$(libj_jarfile): $(JAVA_DBSRCS)
	@test -d $(JAVA_CLASSTOP) || \
	   ($(MKDIR) -p $(JAVA_CLASSTOP) && \
	   $(CHMOD) $(dmode) $(JAVA_CLASSTOP))
	$(JAVAC) -d $(JAVA_CLASSTOP) $(JAVACFLAGS) $(JAVA_DBSRCS)
	$(JAVA) -classpath $(JAVA_CLASSTOP) \
	    com.sleepycat.persist.model.ClassEnhancer $(JAVA_CLASSTOP)
	cd $(JAVA_CLASSTOP) && \
	     $(JAR) cfm ../$(libj_jarfile) ../$(JAVA_MANIFEST) ./com/sleepycat

$(libj_exjarfile): $(libj_jarfile) $(JAVA_EXSRCS)
	@test -d $(JAVA_EXCLASSTOP) || \
	   ($(MKDIR) -p $(JAVA_EXCLASSTOP) && \
		$(CHMOD) $(dmode) $(JAVA_EXCLASSTOP))
	$(JAVAC) -classpath $(libj_jarfile) -d $(JAVA_EXCLASSTOP) \
		$(JAVACFLAGS) $(JAVA_EXSRCS)
	cd $(JAVA_EXCLASSTOP) && $(JAR) cf ../$(libj_exjarfile) .

##################################################
# Utilities
##################################################
berkeley_db_svc: $(RPC_SRV_OBJS) util_log.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    $(RPC_SRV_OBJS) util_log.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

berkeley_db_cxxsvc: $(RPC_CXXSRV_OBJS) util_log.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) \
	    $(RPC_CXXSRV_OBJS) util_log.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

db_archive: db_archive.lo util_sig.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_archive.lo util_sig.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_checkpoint: db_checkpoint.lo util_log.lo util_sig.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_checkpoint.lo util_log.lo util_sig.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_deadlock: db_deadlock.lo util_log.lo util_sig.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_deadlock.lo util_log.lo util_sig.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_dump: db_dump.lo util_cache.lo util_sig.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_dump.lo util_cache.lo util_sig.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_dump185: db_dump185.lo 
	$(CCLINK) -o $@ $(LDFLAGS) db_dump185.lo  $(DB185LIB)
	$(POSTLINK) $@

db_hotbackup: db_hotbackup.lo util_sig.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_hotbackup.lo util_sig.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_load: db_load.lo util_cache.lo util_sig.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_load.lo util_cache.lo util_sig.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_printlog: db_printlog.lo $(PRINT_OBJS) util_sig.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_printlog.lo $(PRINT_OBJS) util_sig.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_recover: db_recover.lo util_sig.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_recover.lo util_sig.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

DBSQL_OBJS=\
          db_sql.lo parse.lo preparser.lo parsefuncs.lo tokenize.lo \
          sqlprintf.lo buildpt.lo utils.lo generate.lo generate_test.lo \
          generation_utils.lo generate_verification.lo hint_comment.lo
db_sql: $(DBSQL_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) $(DBSQL_OBJS) \
            $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_stat: db_stat.lo util_cache.lo util_sig.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_stat.lo util_cache.lo util_sig.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_upgrade: db_upgrade.lo util_sig.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_upgrade.lo util_sig.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_verify: db_verify.lo util_cache.lo util_sig.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_verify.lo util_cache.lo util_sig.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

##################################################
# Library and standard utilities install.
##################################################
library_install: install_setup
library_install: install_include install_lib install_utilities install_docs

uninstall: uninstall_include uninstall_lib uninstall_utilities uninstall_docs

install_setup:
	@test -d $(DESTDIR)$(prefix) || \
	    ($(MKDIR) -p $(DESTDIR)$(prefix) && \
	    $(CHMOD) $(dmode) $(DESTDIR)$(prefix))

INCDOT=	db.h db_cxx.h db_185.h 
install_include:
	@echo "Installing DB include files: $(DESTDIR)$(includedir) ..."
	@test -d $(DESTDIR)$(includedir) || \
	    ($(MKDIR) -p $(DESTDIR)$(includedir) && \
	    $(CHMOD) $(dmode) $(DESTDIR)$(includedir))
	@cd $(DESTDIR)$(includedir) && $(RM) $(INCDOT)
	@for f in $(INCDOT); do \
	    for d in . $(srcdir)/stl ; do \
	        test -f $$d/$$f && \
	            $(CP) -p $$d/$$f $(DESTDIR)$(includedir) ; \
	    done ; \
	    $(CHMOD) $(fmode) $(DESTDIR)$(includedir)/$$f ; \
	done

uninstall_include:
	@cd $(DESTDIR)$(includedir) && $(RM) $(INCDOT)

install_lib:
	@echo "Installing DB library: $(DESTDIR)$(libdir) ..."
	@test -d $(DESTDIR)$(libdir) || \
	    ($(MKDIR) -p $(DESTDIR)$(libdir) && \
	    $(CHMOD) $(dmode) $(DESTDIR)$(libdir))
	@cd $(DESTDIR)$(libdir) && $(RM) $(LIB_INSTALL_FILE_LIST)
	@$(INSTALLER) $(libso_target) $(DESTDIR)$(libdir)
	@(cd $(DESTDIR)$(libdir) && \
	    test -f $(libso) && $(LN) -s $(libso) $(libso_default); \
	    test -f $(libso) && $(LN) -s $(libso) $(libso_major); \
	    test -f $(libxso) && $(LN) -s $(libxso) $(libxso_default); \
	    test -f $(libxso) && $(LN) -s $(libxso) $(libxso_major); \
	    test -f $(libstlso) && $(LN) -s $(libstlso) $(libstlso_default); \
	    test -f $(libstlso) && $(LN) -s $(libstlso) $(libstlso_major); \
	    test -f $(libtso) && $(LN) -s $(libtso) $(libtso_default); \
	    test -f $(libtso) && $(LN) -s $(libtso) $(libtso_major); \
	    test -f $(libjso) && $(LN) -s $(libjso) $(libjso_default); \
	    test -f $(libjso) && $(LN) -s $(libjso) $(libjso_major); \
	    test -f $(libjso) && $(LN) -s $(libjso) $(libjso_g)) || exit 0
	@(test -f $(libj_jarfile) && \
	    $(CP) $(libj_jarfile) $(DESTDIR)$(libdir) && \
	    $(CHMOD) $(fmode) $(DESTDIR)$(libdir)/$(libj_jarfile)) || exit 0

uninstall_lib:
	@cd $(DESTDIR)$(libdir) && $(RM) $(LIB_INSTALL_FILE_LIST)

install_utilities:
	@echo "Installing DB utilities: $(DESTDIR)$(bindir) ..."
	@test -d $(DESTDIR)$(bindir) || \
	    ($(MKDIR) -p $(DESTDIR)$(bindir) && \
	    $(CHMOD) $(dmode) $(DESTDIR)$(bindir))
	@for i in $(UTIL_PROGS); do \
		test -f $$i.exe && i=$$i.exe || i=$$i; \
		$(RM) $(DESTDIR)$(bindir)/$$i; \
		$(INSTALLER) $$i $(DESTDIR)$(bindir)/$$i; \
		$(STRIP) $(DESTDIR)$(bindir)/$$i; \
		$(CHMOD) $(emode) $(DESTDIR)$(bindir)/$$i; \
	done

uninstall_utilities:
	@(cd $(DESTDIR)$(bindir); for i in $(UTIL_PROGS); do \
		$(RM) $$i $$i.exe; \
	done)

DOCLIST=api_reference articles collections csharp gsg gsg_db_rep \
	gsg_txn index.html java license porting programmer_reference

install_docs:
	@echo "Installing documentation: $(DESTDIR)$(docdir) ..."
	@test -d $(DESTDIR)$(docdir) || \
	    ($(MKDIR) -p $(DESTDIR)$(docdir) && \
	    $(CHMOD) $(dmode) $(DESTDIR)$(docdir))
	@cd $(DESTDIR)$(docdir) && $(RM) -r $(DOCLIST)
	@cd $(srcdir)/docs && $(CP) -pr $(DOCLIST) $(DESTDIR)$(docdir)/

uninstall_docs:
	@cd $(DESTDIR)$(docdir) && $(RM) -r $(DOCLIST)

##################################################
# Remaining standard Makefile targets.
##################################################
CLEAN_LIST=\
	StlTxnGuide TxnGuide TxnGuideInMemory bench_001 berkeley_db_cxxsvc \
	berkeley_db_svc db_dump185 db_perf db_repsite db_reptest dbs ex_access \
	ex_apprec ex_btrec ex_dbclient ex_env ex_lock ex_mpool \
	ex_rep_base ex_rep_gsg_repmgr ex_rep_gsg_simple ex_rep_mgr \
	ex_sequence ex_stream ex_thread ex_tpcb \
	example_database_load example_database_read excxx_access \
	excxx_btrec excxx_env excxx_example_database_load \
	excxx_example_database_read excxx_lock excxx_mpool \
	excxx_repquote excxx_repquote_gsg_repmgr excxx_repquote_gsg_simple \
	excxx_sequence excxx_tpcb exstl_access exstl_repquote \
	exstl_tpcb txn_guide test_dbstl txn_guide_inmemory

mostly-clean clean:
	$(RM) -r $(C_OBJS)
	$(RM) -r $(CXX_OBJS) $(JAVA_OBJS) $(STL_OBJS) $(TCL_OBJS)
	$(RM) -r $(RPC_CLIENT_OBJS) $(RPC_SRV_OBJS) $(RPC_CXXSRV_OBJS)
	$(RM) -r $(UTIL_PROGS) *.exe $(CLEAN_LIST)
	$(RM) -r $(JAVA_CLASSTOP) $(JAVA_EXCLASSTOP)
	$(RM) -r $(DB_STL_TEST_OBJS) $(TEST_MICRO_OBJS)
	$(RM) -r tags *.lo *.o *.o.lock *.lo core *.core core.*
	$(RM) -r ALL.OUT.* PARALLEL_TESTDIR.*
	$(RM) -r RUN_LOG RUNQUEUE TESTDIR TESTDIR.A TEST.LIST
	$(RM) -r logtrack_seen.db test_micro test_mutex .libs
	$(RM) -r $(LIB_INSTALL_FILE_LIST)

REALCLEAN_LIST=\
	Makefile clib_port.h confdefs.h config.cache config.log config.status \
	configure.lineno db.h db185_int.h db_185.h db_config.h \
	db_cxx.h db_int.h db_int_def.h include.tcl \
	db_server.h db_server_clnt.c db_server_svc.c db_server_xdr.c \
	gen_db_server.c

distclean maintainer-clean realclean: clean
	$(RM) -r $(REALCLEAN_LIST)
	$(RM) -r libtool

check depend dvi info obj TAGS:
	@echo "make: $@ target not available"

dist rpm rpmbuild:
	@echo "make: $@ target not available" && exit 1

##################################################
# Testers, benchmarks.
##################################################
bench_001.lo: $(srcdir)/examples_c/bench_001.c
	$(CC) $(CFLAGS) $?
bench_001: bench_001.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) bench_001.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

dbs.lo: $(srcdir)/test_server/dbs.c
	$(CC) $(CFLAGS) $?
dbs_am.lo: $(srcdir)/test_server/dbs_am.c
	$(CC) $(CFLAGS) $?
dbs_checkpoint.lo: $(srcdir)/test_server/dbs_checkpoint.c
	$(CC) $(CFLAGS) $?
dbs_debug.lo: $(srcdir)/test_server/dbs_debug.c
	$(CC) $(CFLAGS) $?
dbs_handles.lo: $(srcdir)/test_server/dbs_handles.c
	$(CC) $(CFLAGS) $?
dbs_log.lo: $(srcdir)/test_server/dbs_log.c
	$(CC) $(CFLAGS) $?
dbs_qam.lo: $(srcdir)/test_server/dbs_qam.c
	$(CC) $(CFLAGS) $?
dbs_spawn.lo: $(srcdir)/test_server/dbs_spawn.c
	$(CC) $(CFLAGS) $?
dbs_trickle.lo: $(srcdir)/test_server/dbs_trickle.c
	$(CC) $(CFLAGS) $?
dbs_util.lo: $(srcdir)/test_server/dbs_util.c
	$(CC) $(CFLAGS) $?
dbs_yield.lo: $(srcdir)/test_server/dbs_yield.c
	$(CC) $(CFLAGS) $?
DBS_OBJS=\
	dbs.lo dbs_am.lo dbs_checkpoint.lo dbs_debug.lo dbs_handles.lo \
	dbs_log.lo dbs_qam.lo dbs_spawn.lo dbs_trickle.lo dbs_util.lo \
	dbs_yield.lo
dbs: $(DBS_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(DBS_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

db_perf.lo: $(srcdir)/test_perf/db_perf.c
	$(CC) $(CFLAGS) $?
perf_checkpoint.lo: $(srcdir)/test_perf/perf_checkpoint.c
	$(CC) $(CFLAGS) $?
perf_config.lo: $(srcdir)/test_perf/perf_config.c
	$(CC) $(CFLAGS) $?
perf_dbs.lo: $(srcdir)/test_perf/perf_dbs.c
	$(CC) $(CFLAGS) $?
perf_dead.lo: $(srcdir)/test_perf/perf_dead.c
	$(CC) $(CFLAGS) $?
perf_debug.lo: $(srcdir)/test_perf/perf_debug.c
	$(CC) $(CFLAGS) $?
perf_file.lo: $(srcdir)/test_perf/perf_file.c
	$(CC) $(CFLAGS) $?
perf_key.lo: $(srcdir)/test_perf/perf_key.c
	$(CC) $(CFLAGS) $?
perf_log.lo: $(srcdir)/test_perf/perf_log.c
	$(CC) $(CFLAGS) $?
perf_misc.lo: $(srcdir)/test_perf/perf_misc.c
	$(CC) $(CFLAGS) $?
perf_op.lo: $(srcdir)/test_perf/perf_op.c
	$(CC) $(CFLAGS) $?
perf_parse.lo: $(srcdir)/test_perf/perf_parse.c
	$(CC) $(CFLAGS) $?
perf_rand.lo: $(srcdir)/test_perf/perf_rand.c
	$(CC) $(CFLAGS) $?
perf_spawn.lo: $(srcdir)/test_perf/perf_spawn.c
	$(CC) $(CFLAGS) $?
perf_stat.lo: $(srcdir)/test_perf/perf_stat.c
	$(CC) $(CFLAGS) $?
perf_sync.lo: $(srcdir)/test_perf/perf_sync.c
	$(CC) $(CFLAGS) $?
perf_thread.lo: $(srcdir)/test_perf/perf_thread.c
	$(CC) $(CFLAGS) $?
perf_trickle.lo: $(srcdir)/test_perf/perf_trickle.c
	$(CC) $(CFLAGS) $?
perf_txn.lo: $(srcdir)/test_perf/perf_txn.c
	$(CC) $(CFLAGS) $?
perf_util.lo: $(srcdir)/test_perf/perf_util.c
	$(CC) $(CFLAGS) $?
perf_vx.lo: $(srcdir)/test_perf/perf_vx.c
	$(CC) $(CFLAGS) $?
DBPERF_OBJS=\
	db_perf.lo perf_checkpoint.lo perf_config.lo perf_dbs.lo \
	perf_dead.lo perf_debug.lo perf_file.lo perf_key.lo perf_log.lo \
	perf_misc.lo perf_op.lo perf_parse.lo perf_rand.lo perf_spawn.lo \
	perf_stat.lo perf_sync.lo perf_thread.lo perf_trickle.lo \
	perf_txn.lo perf_util.lo perf_vx.lo util_sig.lo
db_perf: $(DBPERF_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(DBPERF_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

db_repsite.lo: $(srcdir)/test_repmgr/db_repsite.cpp
	$(CXX) $(CXXFLAGS) $?
DBREPSITE_OBJS=db_repsite.lo
db_repsite: $(DBREPSITE_OBJS) $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ \
	   $(LDFLAGS) $(DBREPSITE_OBJS) $(DEF_LIB_CXX) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@ 

db_reptest.lo: $(srcdir)/test_repmgr/db_reptest.c
	$(CC) $(CFLAGS) $?
reptest_am.lo: $(srcdir)/test_repmgr/reptest_am.c
	$(CC) $(CFLAGS) $?
reptest_handles.lo: $(srcdir)/test_repmgr/reptest_handles.c
	$(CC) $(CFLAGS) $?
reptest_spawn.lo: $(srcdir)/test_repmgr/reptest_spawn.c
	$(CC) $(CFLAGS) $?
reptest_util.lo: $(srcdir)/test_repmgr/reptest_util.c
	$(CC) $(CFLAGS) $?
DBREPTEST_OBJS=\
	db_reptest.lo reptest_am.lo reptest_handles.lo \
	reptest_spawn.lo reptest_util.lo 

db_reptest: $(DBREPTEST_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(DBREPTEST_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

test_dbstl.lo: $(srcdir)/test_stl/base/test_dbstl.cpp
	$(CXX) $(STLFLAGS) $?
test_util.lo: $(srcdir)/test_stl/base/test_util.cpp
	$(CXX) $(STLFLAGS) $?
DB_STL_TEST_OBJS=test_dbstl.lo test_util.lo

test_dbstl: $(DB_STL_TEST_OBJS) $(DEF_LIB_CXX) $(DEF_LIB_STL)
	$(CXXLINK) -o $@ $(LDFLAGS) $(DB_STL_TEST_OBJS) \
	    $(DEF_LIB_CXX) $(DEF_LIB_STL) $(LIBS) $(LIBSTLSO_LIBS)
	$(POSTLINK) $@

b_curalloc.lo: $(srcdir)/test_micro/source/b_curalloc.c
	$(CC) $(CFLAGS) -I$(srcdir)/test_micro/source $?
b_curwalk.lo: $(srcdir)/test_micro/source/b_curwalk.c
	$(CC) $(CFLAGS) -I$(srcdir)/test_micro/source $?
b_del.lo: $(srcdir)/test_micro/source/b_del.c
	$(CC) $(CFLAGS) -I$(srcdir)/test_micro/source $?
b_get.lo: $(srcdir)/test_micro/source/b_get.c
	$(CC) $(CFLAGS) -I$(srcdir)/test_micro/source $?
b_inmem.lo: $(srcdir)/test_micro/source/b_inmem.c
	$(CC) $(CFLAGS) -I$(srcdir)/test_micro/source $?
b_latch.lo: $(srcdir)/test_micro/source/b_latch.c
	$(CC) $(CFLAGS) -I$(srcdir)/test_micro/source $?
b_load.lo: $(srcdir)/test_micro/source/b_load.c
	$(CC) $(CFLAGS) -I$(srcdir)/test_micro/source $?
b_open.lo: $(srcdir)/test_micro/source/b_open.c
	$(CC) $(CFLAGS) -I$(srcdir)/test_micro/source $?
b_put.lo: $(srcdir)/test_micro/source/b_put.c
	$(CC) $(CFLAGS) -I$(srcdir)/test_micro/source $?
b_recover.lo: $(srcdir)/test_micro/source/b_recover.c
	$(CC) $(CFLAGS) -I$(srcdir)/test_micro/source $?
b_txn.lo: $(srcdir)/test_micro/source/b_txn.c
	$(CC) $(CFLAGS) -I$(srcdir)/test_micro/source $?
b_txn_write.lo: $(srcdir)/test_micro/source/b_txn_write.c
	$(CC) $(CFLAGS) -I$(srcdir)/test_micro/source $?
b_uname.lo: $(srcdir)/test_micro/source/b_uname.c
	$(CC) $(CFLAGS) -I$(srcdir)/test_micro/source $?
b_util.lo: $(srcdir)/test_micro/source/b_util.c
	$(CC) $(CFLAGS) -I$(srcdir)/test_micro/source $?
b_workload.lo: $(srcdir)/test_micro/source/b_workload.c
	$(CC) $(CFLAGS) -I$(srcdir)/test_micro/source $?
test_micro.lo: $(srcdir)/test_micro/source/test_micro.c
	$(CC) $(CFLAGS) -I$(srcdir)/test_micro/source $?
test_micro: $(TEST_MICRO_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(TEST_MICRO_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

test_mutex.lo: $(srcdir)/mutex/test_mutex.c
	$(CC) $(CFLAGS) $?
test_mutex: test_mutex.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) test_mutex.lo $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

##################################################
# Example programs for C.
##################################################
ex_access.lo: $(srcdir)/examples_c/ex_access.c
	$(CC) $(CFLAGS) $?
ex_access: ex_access.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_access.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_apprec.lo: $(srcdir)/examples_c/ex_apprec/ex_apprec.c
	$(CC) $(CFLAGS) $?
ex_apprec_auto.lo: $(srcdir)/examples_c/ex_apprec/ex_apprec_auto.c
	$(CC) $(CFLAGS) $?
ex_apprec_autop.lo: $(srcdir)/examples_c/ex_apprec/ex_apprec_autop.c
	$(CC) $(CFLAGS) $?
ex_apprec_rec.lo: $(srcdir)/examples_c/ex_apprec/ex_apprec_rec.c
	$(CC) $(CFLAGS) $?
EX_APPREC_OBJS=\
	ex_apprec.lo ex_apprec_auto.lo ex_apprec_autop.lo ex_apprec_rec.lo
ex_apprec: $(EX_APPREC_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(EX_APPREC_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)

ex_btrec.lo: $(srcdir)/examples_c/ex_btrec.c
	$(CC) $(CFLAGS) $?
ex_btrec: ex_btrec.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_btrec.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_dbclient.lo: $(srcdir)/examples_c/ex_dbclient.c
	$(CC) $(CFLAGS) $?
ex_dbclient: ex_dbclient.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_dbclient.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_env.lo: $(srcdir)/examples_c/ex_env.c
	$(CC) $(CFLAGS) $?
ex_env: ex_env.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_env.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_lock.lo: $(srcdir)/examples_c/ex_lock.c
	$(CC) $(CFLAGS) $?
ex_lock: ex_lock.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_lock.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_mpool.lo: $(srcdir)/examples_c/ex_mpool.c
	$(CC) $(CFLAGS) $?
ex_mpool: ex_mpool.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_mpool.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

rep_base.lo: $(srcdir)/examples_c/ex_rep/base/rep_base.c
	$(CC) $(CFLAGS) $?
rep_common.lo: $(srcdir)/examples_c/ex_rep/common/rep_common.c
	$(CC) $(CFLAGS) $?
rep_msg.lo: $(srcdir)/examples_c/ex_rep/base/rep_msg.c
	$(CC) $(CFLAGS) $?
rep_net.lo: $(srcdir)/examples_c/ex_rep/base/rep_net.c
	$(CC) $(CFLAGS) $?
EX_REP_BASE_OBJS=\
	rep_base.lo rep_common.lo rep_msg.lo rep_net.lo
ex_rep_base: $(EX_REP_BASE_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(EX_REP_BASE_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

simple_txn.lo: $(srcdir)/examples_c/ex_rep_gsg/simple_txn.c
	$(CC) $(CFLAGS) $?
ex_rep_gsg_simple: simple_txn.lo $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) simple_txn.lo $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

rep_mgr_gsg.lo: $(srcdir)/examples_c/ex_rep_gsg/rep_mgr_gsg.c
	$(CC) $(CFLAGS) $?
ex_rep_gsg_repmgr: rep_mgr_gsg.lo $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) rep_mgr_gsg.lo $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

rep_mgr.lo: $(srcdir)/examples_c/ex_rep/mgr/rep_mgr.c
	$(CC) $(CFLAGS) $?
EX_REP_MGR_OBJS=\
	rep_common.lo rep_mgr.lo
ex_rep_mgr: $(EX_REP_MGR_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(EX_REP_MGR_OBJS) $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

ex_sequence.lo: $(srcdir)/examples_c/ex_sequence.c
	$(CC) $(CFLAGS) $?
ex_sequence: ex_sequence.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_sequence.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_stream.lo: $(srcdir)/examples_c/ex_stream.c
	$(CC) $(CFLAGS) $?
ex_stream: ex_stream.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_stream.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_thread.lo: $(srcdir)/examples_c/ex_thread.c
	$(CC) $(CFLAGS) $?
ex_thread: ex_thread.lo $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) ex_thread.lo $(DEF_LIB) $(TEST_LIBS) $(LIBS)
	$(POSTLINK) $@

ex_tpcb.lo: $(srcdir)/examples_c/ex_tpcb.c
	$(CC) $(CFLAGS) $?
ex_tpcb: ex_tpcb.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_tpcb.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

gettingstarted_common.lo: \
    $(srcdir)/examples_c/getting_started/gettingstarted_common.c
	$(CC) -I$(srcdir)/examples_c/getting_started $(CFLAGS) $?
example_database_load.lo: \
    $(srcdir)/examples_c/getting_started/example_database_load.c
	$(CC) $(CFLAGS) $?
example_database_read.lo: \
    $(srcdir)/examples_c/getting_started/example_database_read.c
	$(CC) $(CFLAGS) $?
example_database_load: example_database_load.lo gettingstarted_common.lo \
    $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    example_database_load.lo gettingstarted_common.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@
example_database_read: example_database_read.lo gettingstarted_common.lo \
    $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    example_database_read.lo gettingstarted_common.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

txn_guide_inmemory.lo: $(srcdir)/examples_c/txn_guide/txn_guide_inmemory.c
	$(CC) $(CFLAGS) $?
txn_guide_inmemory: txn_guide_inmemory.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) txn_guide_inmemory.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

txn_guide.lo: $(srcdir)/examples_c/txn_guide/txn_guide.c
	$(CC) $(CFLAGS) $?
txn_guide: txn_guide.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) txn_guide.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

##################################################
# Example programs for C++.
##################################################
AccessExample.lo: $(srcdir)/examples_cxx/AccessExample.cpp
	$(CXX) $(CXXFLAGS) $?
excxx_access: AccessExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) AccessExample.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

BtRecExample.lo: $(srcdir)/examples_cxx/BtRecExample.cpp
	$(CXX) $(CXXFLAGS) $?
excxx_btrec: BtRecExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) BtRecExample.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

EnvExample.lo: $(srcdir)/examples_cxx/EnvExample.cpp
	$(CXX) $(CXXFLAGS) $?
excxx_env: EnvExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) EnvExample.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

LockExample.lo: $(srcdir)/examples_cxx/LockExample.cpp
	$(CXX) $(CXXFLAGS) $?
excxx_lock: LockExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) LockExample.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

MpoolExample.lo: $(srcdir)/examples_cxx/MpoolExample.cpp
	$(CXX) $(CXXFLAGS) $?
excxx_mpool: MpoolExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) MpoolExample.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

RepQuoteExample.lo: $(srcdir)/examples_cxx/excxx_repquote/RepQuoteExample.cpp
	$(CXX) -I$(srcdir)/examples_cxx/excxx_repquote $(CXXFLAGS) $?
RepConfigInfo.lo: $(srcdir)/examples_cxx/excxx_repquote/RepConfigInfo.cpp
	$(CXX) -I$(srcdir)/examples_cxx/excxx_repquote $(CXXFLAGS) $?
excxx_repquote: RepQuoteExample.lo RepConfigInfo.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) \
	    RepQuoteExample.lo RepConfigInfo.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

RepMgrGSG.lo: $(srcdir)/examples_cxx/excxx_repquote_gsg/RepMgrGSG.cpp
	$(CXX) -I$(srcdir)/examples_cxx/excxx_repquote_gsg $(CXXFLAGS) $?
excxx_repquote_gsg_repmgr: RepMgrGSG.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) RepMgrGSG.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

SimpleTxn.lo: $(srcdir)/examples_cxx/excxx_repquote_gsg/SimpleTxn.cpp
	$(CXX) -I$(srcdir)/examples_cxx/excxx_repquote_gsg $(CXXFLAGS) $?
excxx_repquote_gsg_simple: SimpleTxn.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) SimpleTxn.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

SequenceExample.lo: $(srcdir)/examples_cxx/SequenceExample.cpp
	$(CXX) $(CXXFLAGS) $?
excxx_sequence: SequenceExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) SequenceExample.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

TpcbExample.lo: $(srcdir)/examples_cxx/TpcbExample.cpp
	$(CXX) $(CXXFLAGS) $?
excxx_tpcb: TpcbExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) TpcbExample.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

excxx_example_database_load.lo: \
    $(srcdir)/examples_cxx/getting_started/excxx_example_database_load.cpp
	$(CXX) -I$(srcdir)/examples_cxx/getting_started $(CXXFLAGS) $?
excxx_example_database_read.lo: \
    $(srcdir)/examples_cxx/getting_started/excxx_example_database_read.cpp
	$(CXX) -I$(srcdir)/examples_cxx/getting_started $(CXXFLAGS) $?
MyDb.lo: $(srcdir)/examples_cxx/getting_started/MyDb.cpp
	$(CXX) -I$(srcdir)/examples_cxx/getting_started $(CXXFLAGS) $?
excxx_example_database_load: \
    excxx_example_database_load.lo MyDb.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) \
	    excxx_example_database_load.lo MyDb.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@
excxx_example_database_read: \
    excxx_example_database_read.lo MyDb.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) \
	    excxx_example_database_read.lo MyDb.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

TxnGuideInMemory.lo: $(srcdir)/examples_cxx/txn_guide/TxnGuideInMemory.cpp
	$(CXX) $(CXXFLAGS) $?
TxnGuideInMemory: TxnGuideInMemory.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) TxnGuideInMemory.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

TxnGuide.lo: $(srcdir)/examples_cxx/txn_guide/TxnGuide.cpp
	$(CXX) $(CXXFLAGS) $?
TxnGuide: TxnGuide.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) TxnGuide.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

##################################################
# Example programs for STL.
##################################################
StlAccessExample.lo: $(srcdir)/examples_stl/StlAccessExample.cpp
	$(CXX) $(STLFLAGS) $?
exstl_access: StlAccessExample.lo $(DEF_LIB_STL)
	$(CXXLINK) -o $@ $(LDFLAGS) StlAccessExample.lo $(DEF_LIB_STL) $(LIBS)
	$(POSTLINK) $@

StlRepQuoteExample.lo: $(srcdir)/examples_stl/repquote/StlRepQuoteExample.cpp
	$(CXX) -I$(srcdir)/examples_stl/repquote $(STLFLAGS) $?
StlRepConfigInfo.lo: $(srcdir)/examples_stl/repquote/StlRepConfigInfo.cpp
	$(CXX) -I$(srcdir)/examples_stl/repquote $(STLFLAGS) $?
exstl_repquote: StlRepQuoteExample.lo StlRepConfigInfo.lo $(DEF_LIB_STL)
	$(CXXLINK) -o $@ $(LDFLAGS) \
	    StlRepQuoteExample.lo StlRepConfigInfo.lo $(DEF_LIB_STL) $(LIBS)
	$(POSTLINK) $@

StlTpcbExample.lo: $(srcdir)/examples_stl/StlTpcbExample.cpp
	$(CXX) $(STLFLAGS) $?
exstl_tpcb: StlTpcbExample.lo $(DEF_LIB_STL)
	$(CXXLINK) -o $@ $(LDFLAGS) StlTpcbExample.lo $(DEF_LIB_STL) $(LIBS)
	$(POSTLINK) $@

StlTransactionGuideExample.lo: $(srcdir)/examples_stl/StlTransactionGuideExample.cpp
	$(CXX) $(STLFLAGS) $?
StlTxnGuide: StlTransactionGuideExample.lo $(DEF_LIB_STL)
	$(CXXLINK) -o $@ $(LDFLAGS) StlTransactionGuideExample.lo $(DEF_LIB_STL) $(LIBS)
	$(POSTLINK) $@

##################################################
# C API build rules.
##################################################
aes_method.lo: $(srcdir)/crypto/aes_method.c
	 $(CC) $(CFLAGS) $?
bt_compare.lo: $(srcdir)/btree/bt_compare.c
	 $(CC) $(CFLAGS) $?
bt_compress.lo: $(srcdir)/btree/bt_compress.c
	 $(CC) $(CFLAGS) $?
bt_conv.lo: $(srcdir)/btree/bt_conv.c
	 $(CC) $(CFLAGS) $?
bt_curadj.lo: $(srcdir)/btree/bt_curadj.c
	 $(CC) $(CFLAGS) $?
bt_cursor.lo: $(srcdir)/btree/bt_cursor.c
	 $(CC) $(CFLAGS) $?
bt_delete.lo: $(srcdir)/btree/bt_delete.c
	 $(CC) $(CFLAGS) $?
bt_method.lo: $(srcdir)/btree/bt_method.c
	 $(CC) $(CFLAGS) $?
bt_open.lo: $(srcdir)/btree/bt_open.c
	 $(CC) $(CFLAGS) $?
bt_put.lo: $(srcdir)/btree/bt_put.c
	 $(CC) $(CFLAGS) $?
bt_rec.lo: $(srcdir)/btree/bt_rec.c
	 $(CC) $(CFLAGS) $?
bt_reclaim.lo: $(srcdir)/btree/bt_reclaim.c
	 $(CC) $(CFLAGS) $?
bt_recno.lo: $(srcdir)/btree/bt_recno.c
	 $(CC) $(CFLAGS) $?
bt_rsearch.lo: $(srcdir)/btree/bt_rsearch.c
	 $(CC) $(CFLAGS) $?
bt_search.lo: $(srcdir)/btree/bt_search.c
	 $(CC) $(CFLAGS) $?
bt_split.lo: $(srcdir)/btree/bt_split.c
	 $(CC) $(CFLAGS) $?
bt_stat.lo: $(srcdir)/btree/bt_stat.c
	 $(CC) $(CFLAGS) $?
bt_compact.lo: $(srcdir)/btree/bt_compact.c
	 $(CC) $(CFLAGS) $?
bt_upgrade.lo: $(srcdir)/btree/bt_upgrade.c
	 $(CC) $(CFLAGS) $?
bt_verify.lo: $(srcdir)/btree/bt_verify.c
	 $(CC) $(CFLAGS) $?
btree_auto.lo: $(srcdir)/btree/btree_auto.c
	 $(CC) $(CFLAGS) $?
btree_autop.lo: $(srcdir)/btree/btree_autop.c
	 $(CC) $(CFLAGS) $?
crdel_auto.lo: $(srcdir)/db/crdel_auto.c
	 $(CC) $(CFLAGS) $?
crdel_autop.lo: $(srcdir)/db/crdel_autop.c
	 $(CC) $(CFLAGS) $?
crdel_rec.lo: $(srcdir)/db/crdel_rec.c
	 $(CC) $(CFLAGS) $?
crypto.lo: $(srcdir)/crypto/crypto.c
	 $(CC) $(CFLAGS) $?
crypto_stub.lo: $(srcdir)/common/crypto_stub.c
	 $(CC) $(CFLAGS) $?
db185.lo: $(srcdir)/db185/db185.c
	 $(CC) $(CFLAGS) $?
db.lo: $(srcdir)/db/db.c
	 $(CC) $(CFLAGS) $?
db_am.lo: $(srcdir)/db/db_am.c
	 $(CC) $(CFLAGS) $?
db_auto.lo: $(srcdir)/db/db_auto.c
	 $(CC) $(CFLAGS) $?
db_autop.lo: $(srcdir)/db/db_autop.c
	 $(CC) $(CFLAGS) $?
db_byteorder.lo: $(srcdir)/common/db_byteorder.c
	 $(CC) $(CFLAGS) $?
db_cam.lo: $(srcdir)/db/db_cam.c
	 $(CC) $(CFLAGS) $?
db_cds.lo: $(srcdir)/db/db_cds.c
	 $(CC) $(CFLAGS) $?
db_compint.lo: $(srcdir)/common/db_compint.c
	 $(CC) $(CFLAGS) $?
db_conv.lo: $(srcdir)/db/db_conv.c
	 $(CC) $(CFLAGS) $?
db_dispatch.lo: $(srcdir)/db/db_dispatch.c
	 $(CC) $(CFLAGS) $?
db_dup.lo: $(srcdir)/db/db_dup.c
	 $(CC) $(CFLAGS) $?
db_err.lo: $(srcdir)/common/db_err.c
	 $(CC) $(CFLAGS) $?
db_getlong.lo: $(srcdir)/common/db_getlong.c
	 $(CC) $(CFLAGS) $?
db_idspace.lo: $(srcdir)/common/db_idspace.c
	 $(CC) $(CFLAGS) $?
db_iface.lo: $(srcdir)/db/db_iface.c
	 $(CC) $(CFLAGS) $?
db_join.lo: $(srcdir)/db/db_join.c
	 $(CC) $(CFLAGS) $?
db_log2.lo: $(srcdir)/common/db_log2.c
	 $(CC) $(CFLAGS) $?
db_meta.lo: $(srcdir)/db/db_meta.c
	 $(CC) $(CFLAGS) $?
db_method.lo: $(srcdir)/db/db_method.c
	 $(CC) $(CFLAGS) $?
db_open.lo: $(srcdir)/db/db_open.c
	 $(CC) $(CFLAGS) $?
db_overflow.lo: $(srcdir)/db/db_overflow.c
	 $(CC) $(CFLAGS) $?
db_ovfl_vrfy.lo: $(srcdir)/db/db_ovfl_vrfy.c
	 $(CC) $(CFLAGS) $?
db_pr.lo: $(srcdir)/db/db_pr.c
	 $(CC) $(CFLAGS) $?
db_rec.lo: $(srcdir)/db/db_rec.c
	 $(CC) $(CFLAGS) $?
db_reclaim.lo: $(srcdir)/db/db_reclaim.c
	 $(CC) $(CFLAGS) $?
db_rename.lo: $(srcdir)/db/db_rename.c
	 $(CC) $(CFLAGS) $?
db_remove.lo: $(srcdir)/db/db_remove.c
	 $(CC) $(CFLAGS) $?
db_ret.lo: $(srcdir)/db/db_ret.c
	 $(CC) $(CFLAGS) $?
db_setid.lo: $(srcdir)/db/db_setid.c
	 $(CC) $(CFLAGS) $?
db_setlsn.lo: $(srcdir)/db/db_setlsn.c
	 $(CC) $(CFLAGS) $?
db_shash.lo: $(srcdir)/common/db_shash.c
	 $(CC) $(CFLAGS) $?
db_sort_multiple.lo: $(srcdir)/db/db_sort_multiple.c
	 $(CC) $(CFLAGS) $?
db_stati.lo: $(srcdir)/db/db_stati.c
	 $(CC) $(CFLAGS) $?
db_truncate.lo: $(srcdir)/db/db_truncate.c
	 $(CC) $(CFLAGS) $?
db_upg.lo: $(srcdir)/db/db_upg.c
	 $(CC) $(CFLAGS) $?
db_upg_opd.lo: $(srcdir)/db/db_upg_opd.c
	 $(CC) $(CFLAGS) $?
db_vrfy.lo: $(srcdir)/db/db_vrfy.c
	 $(CC) $(CFLAGS) $?
db_vrfyutil.lo: $(srcdir)/db/db_vrfyutil.c
	 $(CC) $(CFLAGS) $?
db_vrfy_stub.lo: $(srcdir)/db/db_vrfy_stub.c
	$(CC) $(CFLAGS) $?
dbm.lo: $(srcdir)/dbm/dbm.c
	 $(CC) $(CFLAGS) $?
dbreg.lo: $(srcdir)/dbreg/dbreg.c
	 $(CC) $(CFLAGS) $?
dbreg_auto.lo: $(srcdir)/dbreg/dbreg_auto.c
	 $(CC) $(CFLAGS) $?
dbreg_autop.lo: $(srcdir)/dbreg/dbreg_autop.c
	 $(CC) $(CFLAGS) $?
dbreg_rec.lo: $(srcdir)/dbreg/dbreg_rec.c
	 $(CC) $(CFLAGS) $?
dbreg_stat.lo: $(srcdir)/dbreg/dbreg_stat.c
	 $(CC) $(CFLAGS) $?
dbreg_util.lo: $(srcdir)/dbreg/dbreg_util.c
	 $(CC) $(CFLAGS) $?
dbt.lo: $(srcdir)/common/dbt.c
	 $(CC) $(CFLAGS) $?
env_alloc.lo: $(srcdir)/env/env_alloc.c
	 $(CC) $(CFLAGS) $?
env_config.lo: $(srcdir)/env/env_config.c
	 $(CC) $(CFLAGS) $?
env_failchk.lo: $(srcdir)/env/env_failchk.c
	 $(CC) $(CFLAGS) $?
env_file.lo: $(srcdir)/env/env_file.c
	 $(CC) $(CFLAGS) $?
env_globals.lo: $(srcdir)/env/env_globals.c
	 $(CC) $(CFLAGS) $?
env_method.lo: $(srcdir)/env/env_method.c
	 $(CC) $(CFLAGS) $?
env_name.lo: $(srcdir)/env/env_name.c
	 $(CC) $(CFLAGS) $?
env_open.lo: $(srcdir)/env/env_open.c
	 $(CC) $(CFLAGS) $?
env_recover.lo: $(srcdir)/env/env_recover.c
	 $(CC) $(CFLAGS) $?
env_region.lo: $(srcdir)/env/env_region.c
	 $(CC) $(CFLAGS) $?
env_register.lo: $(srcdir)/env/env_register.c
	 $(CC) $(CFLAGS) $?
env_sig.lo: $(srcdir)/env/env_sig.c
	 $(CC) $(CFLAGS) $?
env_stat.lo: $(srcdir)/env/env_stat.c
	 $(CC) $(CFLAGS) $?
fileops_auto.lo: $(srcdir)/fileops/fileops_auto.c
	 $(CC) $(CFLAGS) $?
fileops_autop.lo: $(srcdir)/fileops/fileops_autop.c
	 $(CC) $(CFLAGS) $?
fop_basic.lo: $(srcdir)/fileops/fop_basic.c
	 $(CC) $(CFLAGS) $?
fop_rec.lo: $(srcdir)/fileops/fop_rec.c
	 $(CC) $(CFLAGS) $?
fop_util.lo: $(srcdir)/fileops/fop_util.c
	 $(CC) $(CFLAGS) $?
hash.lo: $(srcdir)/hash/hash.c
	 $(CC) $(CFLAGS) $?
hash_auto.lo: $(srcdir)/hash/hash_auto.c
	 $(CC) $(CFLAGS) $?
hash_autop.lo: $(srcdir)/hash/hash_autop.c
	 $(CC) $(CFLAGS) $?
hash_conv.lo: $(srcdir)/hash/hash_conv.c
	 $(CC) $(CFLAGS) $?
hash_dup.lo: $(srcdir)/hash/hash_dup.c
	 $(CC) $(CFLAGS) $?
hash_func.lo: $(srcdir)/hash/hash_func.c
	 $(CC) $(CFLAGS) $?
hash_meta.lo: $(srcdir)/hash/hash_meta.c
	 $(CC) $(CFLAGS) $?
hash_method.lo: $(srcdir)/hash/hash_method.c
	 $(CC) $(CFLAGS) $?
hash_open.lo: $(srcdir)/hash/hash_open.c
	 $(CC) $(CFLAGS) $?
hash_page.lo: $(srcdir)/hash/hash_page.c
	 $(CC) $(CFLAGS) $?
hash_rec.lo: $(srcdir)/hash/hash_rec.c
	 $(CC) $(CFLAGS) $?
hash_reclaim.lo: $(srcdir)/hash/hash_reclaim.c
	 $(CC) $(CFLAGS) $?
hash_stat.lo: $(srcdir)/hash/hash_stat.c
	 $(CC) $(CFLAGS) $?
hash_stub.lo: $(srcdir)/hash/hash_stub.c
	 $(CC) $(CFLAGS) $?
hash_upgrade.lo: $(srcdir)/hash/hash_upgrade.c
	 $(CC) $(CFLAGS) $?
hash_verify.lo: $(srcdir)/hash/hash_verify.c
	 $(CC) $(CFLAGS) $?
hmac.lo: $(srcdir)/hmac/hmac.c
	 $(CC) $(CFLAGS) $?
hsearch.lo: $(srcdir)/hsearch/hsearch.c
	 $(CC) $(CFLAGS) $?
lock.lo: $(srcdir)/lock/lock.c
	 $(CC) $(CFLAGS) $?
lock_deadlock.lo:$(srcdir)/lock/lock_deadlock.c
	 $(CC) $(CFLAGS) $?
lock_failchk.lo:$(srcdir)/lock/lock_failchk.c
	 $(CC) $(CFLAGS) $?
lock_id.lo:$(srcdir)/lock/lock_id.c
	 $(CC) $(CFLAGS) $?
lock_list.lo:$(srcdir)/lock/lock_list.c
	 $(CC) $(CFLAGS) $?
lock_method.lo:$(srcdir)/lock/lock_method.c
	 $(CC) $(CFLAGS) $?
lock_region.lo:$(srcdir)/lock/lock_region.c
	 $(CC) $(CFLAGS) $?
lock_stat.lo:$(srcdir)/lock/lock_stat.c
	 $(CC) $(CFLAGS) $?
lock_stub.lo: $(srcdir)/lock/lock_stub.c
	 $(CC) $(CFLAGS) $?
lock_timer.lo:$(srcdir)/lock/lock_timer.c
	 $(CC) $(CFLAGS) $?
lock_util.lo:$(srcdir)/lock/lock_util.c
	 $(CC) $(CFLAGS) $?
log.lo: $(srcdir)/log/log.c
	 $(CC) $(CFLAGS) $?
log_archive.lo: $(srcdir)/log/log_archive.c
	 $(CC) $(CFLAGS) $?
log_compare.lo: $(srcdir)/log/log_compare.c
	 $(CC) $(CFLAGS) $?
log_debug.lo: $(srcdir)/log/log_debug.c
	 $(CC) $(CFLAGS) $?
log_get.lo: $(srcdir)/log/log_get.c
	 $(CC) $(CFLAGS) $?
log_method.lo: $(srcdir)/log/log_method.c
	 $(CC) $(CFLAGS) $?
log_put.lo: $(srcdir)/log/log_put.c
	 $(CC) $(CFLAGS) $?
log_stat.lo: $(srcdir)/log/log_stat.c
	 $(CC) $(CFLAGS) $?
mkpath.lo: $(srcdir)/common/mkpath.c
	 $(CC) $(CFLAGS) $?
mp_alloc.lo: $(srcdir)/mp/mp_alloc.c
	 $(CC) $(CFLAGS) $?
mp_bh.lo: $(srcdir)/mp/mp_bh.c
	 $(CC) $(CFLAGS) $?
mp_fget.lo: $(srcdir)/mp/mp_fget.c
	 $(CC) $(CFLAGS) $?
mp_fmethod.lo: $(srcdir)/mp/mp_fmethod.c
	 $(CC) $(CFLAGS) $?
mp_fopen.lo: $(srcdir)/mp/mp_fopen.c
	 $(CC) $(CFLAGS) $?
mp_fput.lo: $(srcdir)/mp/mp_fput.c
	 $(CC) $(CFLAGS) $?
mp_fset.lo: $(srcdir)/mp/mp_fset.c
	 $(CC) $(CFLAGS) $?
mp_method.lo: $(srcdir)/mp/mp_method.c
	 $(CC) $(CFLAGS) $?
mp_mvcc.lo: $(srcdir)/mp/mp_mvcc.c
	 $(CC) $(CFLAGS) $?
mp_region.lo: $(srcdir)/mp/mp_region.c
	 $(CC) $(CFLAGS) $?
mp_register.lo: $(srcdir)/mp/mp_register.c
	 $(CC) $(CFLAGS) $?
mp_resize.lo: $(srcdir)/mp/mp_resize.c
	 $(CC) $(CFLAGS) $?
mp_stat.lo: $(srcdir)/mp/mp_stat.c
	 $(CC) $(CFLAGS) $?
mp_sync.lo: $(srcdir)/mp/mp_sync.c
	 $(CC) $(CFLAGS) $?
mp_trickle.lo: $(srcdir)/mp/mp_trickle.c
	 $(CC) $(CFLAGS) $?
mt19937db.lo: $(srcdir)/crypto/mersenne/mt19937db.c
	 $(CC) $(CFLAGS) $?
mut_alloc.lo: $(srcdir)/mutex/mut_alloc.c
	 $(CC) $(CFLAGS) $?
mut_failchk.lo: $(srcdir)/mutex/mut_failchk.c
	 $(CC) $(CFLAGS) $?
mut_fcntl.lo: $(srcdir)/mutex/mut_fcntl.c
	 $(CC) $(CFLAGS) $?
mut_method.lo: $(srcdir)/mutex/mut_method.c
	 $(CC) $(CFLAGS) $?
mut_pthread.lo: $(srcdir)/mutex/mut_pthread.c
	 $(CC) $(CFLAGS) $?
mut_region.lo: $(srcdir)/mutex/mut_region.c
	 $(CC) $(CFLAGS) $?
mut_stat.lo: $(srcdir)/mutex/mut_stat.c
	 $(CC) $(CFLAGS) $?
mut_stub.lo: $(srcdir)/mutex/mut_stub.c
	 $(CC) $(CFLAGS) $?
mut_tas.lo: $(srcdir)/mutex/mut_tas.c
	 $(CC) $(CFLAGS) $?
mut_win32.lo: $(srcdir)/mutex/mut_win32.c
	 $(CC) $(CFLAGS) $?
openflags.lo: $(srcdir)/common/openflags.c
	 $(CC) $(CFLAGS) $?
os_abs.lo: $(srcdir)/os/os_abs.c
	 $(CC) $(CFLAGS) $?
os_abort.lo: $(srcdir)/os/os_abort.c
	 $(CC) $(CFLAGS) $?
os_addrinfo.lo: $(srcdir)/os/os_addrinfo.c
	$(CC) $(CFLAGS) $?
os_alloc.lo: $(srcdir)/os/os_alloc.c
	 $(CC) $(CFLAGS) $?
os_clock.lo: $(srcdir)/os/os_clock.c
	 $(CC) $(CFLAGS) $?
os_config.lo: $(srcdir)/os/os_config.c
	 $(CC) $(CFLAGS) $?
os_cpu.lo: $(srcdir)/os/os_cpu.c
	 $(CC) $(CFLAGS) $?
os_ctime.lo: $(srcdir)/os/os_ctime.c
	 $(CC) $(CFLAGS) $?
os_dir.lo: $(srcdir)/os/os_dir.c
	 $(CC) $(CFLAGS) $?
os_errno.lo: $(srcdir)/os/os_errno.c
	 $(CC) $(CFLAGS) $?
os_fid.lo: $(srcdir)/os/os_fid.c
	 $(CC) $(CFLAGS) $?
os_flock.lo: $(srcdir)/os/os_flock.c
	 $(CC) $(CFLAGS) $?
os_fsync.lo: $(srcdir)/os/os_fsync.c
	 $(CC) $(CFLAGS) $?
os_getenv.lo: $(srcdir)/os/os_getenv.c
	 $(CC) $(CFLAGS) $?
os_handle.lo: $(srcdir)/os/os_handle.c
	 $(CC) $(CFLAGS) $?
os_map.lo: $(srcdir)/os/os_map.c
	 $(CC) $(CFLAGS) $?
os_method.lo: $(srcdir)/common/os_method.c
	 $(CC) $(CFLAGS) $?
os_mkdir.lo: $(srcdir)/os/os_mkdir.c
	 $(CC) $(CFLAGS) $?
os_open.lo: $(srcdir)/os/os_open.c
	 $(CC) $(CFLAGS) $?
os_pid.lo: $(srcdir)/os/os_pid.c
	 $(CC) $(CFLAGS) $?
os_qnx_fsync.lo: $(srcdir)/os_qnx/os_qnx_fsync.c
	 $(CC) $(CFLAGS) $?
os_qnx_open.lo: $(srcdir)/os_qnx/os_qnx_open.c
	 $(CC) $(CFLAGS) $?
os_rename.lo: $(srcdir)/os/os_rename.c
	 $(CC) $(CFLAGS) $?
os_root.lo: $(srcdir)/os/os_root.c
	 $(CC) $(CFLAGS) $?
os_rpath.lo: $(srcdir)/os/os_rpath.c
	 $(CC) $(CFLAGS) $?
os_rw.lo: $(srcdir)/os/os_rw.c
	 $(CC) $(CFLAGS) $?
os_seek.lo: $(srcdir)/os/os_seek.c
	 $(CC) $(CFLAGS) $?
os_stack.lo: $(srcdir)/os/os_stack.c
	 $(CC) $(CFLAGS) $?
os_stat.lo: $(srcdir)/os/os_stat.c
	 $(CC) $(CFLAGS) $?
os_tmpdir.lo: $(srcdir)/os/os_tmpdir.c
	 $(CC) $(CFLAGS) $?
os_truncate.lo: $(srcdir)/os/os_truncate.c
	 $(CC) $(CFLAGS) $?
os_uid.lo: $(srcdir)/os/os_uid.c
	 $(CC) $(CFLAGS) $?
os_unlink.lo: $(srcdir)/os/os_unlink.c
	 $(CC) $(CFLAGS) $?
os_yield.lo: $(srcdir)/os/os_yield.c
	 $(CC) $(CFLAGS) $?
partition.lo: $(srcdir)/db/partition.c
	 $(CC) $(CFLAGS) $?
qam.lo: $(srcdir)/qam/qam.c
	 $(CC) $(CFLAGS) $?
qam_auto.lo: $(srcdir)/qam/qam_auto.c
	 $(CC) $(CFLAGS) $?
qam_autop.lo: $(srcdir)/qam/qam_autop.c
	 $(CC) $(CFLAGS) $?
qam_conv.lo: $(srcdir)/qam/qam_conv.c
	 $(CC) $(CFLAGS) $?
qam_files.lo: $(srcdir)/qam/qam_files.c
	 $(CC) $(CFLAGS) $?
qam_method.lo: $(srcdir)/qam/qam_method.c
	 $(CC) $(CFLAGS) $?
qam_open.lo: $(srcdir)/qam/qam_open.c
	 $(CC) $(CFLAGS) $?
qam_rec.lo: $(srcdir)/qam/qam_rec.c
	 $(CC) $(CFLAGS) $?
qam_stat.lo: $(srcdir)/qam/qam_stat.c
	 $(CC) $(CFLAGS) $?
qam_stub.lo: $(srcdir)/qam/qam_stub.c
	 $(CC) $(CFLAGS) $?
qam_upgrade.lo: $(srcdir)/qam/qam_upgrade.c
	 $(CC) $(CFLAGS) $?
qam_verify.lo: $(srcdir)/qam/qam_verify.c
	 $(CC) $(CFLAGS) $?
rep_auto.lo: $(srcdir)/rep/rep_auto.c
	 $(CC) $(CFLAGS) $?
rep_backup.lo: $(srcdir)/rep/rep_backup.c
	 $(CC) $(CFLAGS) $?
rep_elect.lo: $(srcdir)/rep/rep_elect.c
	 $(CC) $(CFLAGS) $?
rep_lease.lo: $(srcdir)/rep/rep_lease.c
	 $(CC) $(CFLAGS) $?
rep_log.lo: $(srcdir)/rep/rep_log.c
	 $(CC) $(CFLAGS) $?
rep_method.lo: $(srcdir)/rep/rep_method.c
	 $(CC) $(CFLAGS) $?
rep_record.lo: $(srcdir)/rep/rep_record.c
	 $(CC) $(CFLAGS) $?
rep_region.lo: $(srcdir)/rep/rep_region.c
	 $(CC) $(CFLAGS) $?
rep_stub.lo: $(srcdir)/rep/rep_stub.c
	 $(CC) $(CFLAGS) $?
rep_stat.lo: $(srcdir)/rep/rep_stat.c
	 $(CC) $(CFLAGS) $?
rep_util.lo: $(srcdir)/rep/rep_util.c
	 $(CC) $(CFLAGS) $?
rep_verify.lo: $(srcdir)/rep/rep_verify.c
	 $(CC) $(CFLAGS) $?
repmgr_auto.lo: $(srcdir)/repmgr/repmgr_auto.c
	 $(CC) $(CFLAGS) $?
repmgr_elect.lo: $(srcdir)/repmgr/repmgr_elect.c
	$(CC) $(CFLAGS) $?
repmgr_method.lo: $(srcdir)/repmgr/repmgr_method.c
	$(CC) $(CFLAGS) $?
repmgr_msg.lo: $(srcdir)/repmgr/repmgr_msg.c
	$(CC) $(CFLAGS) $?
repmgr_net.lo: $(srcdir)/repmgr/repmgr_net.c
	$(CC) $(CFLAGS) $?
repmgr_posix.lo: $(srcdir)/repmgr/repmgr_posix.c
	$(CC) $(CFLAGS) $?
repmgr_queue.lo: $(srcdir)/repmgr/repmgr_queue.c
	$(CC) $(CFLAGS) $?
repmgr_sel.lo: $(srcdir)/repmgr/repmgr_sel.c
	$(CC) $(CFLAGS) $?
repmgr_stat.lo: $(srcdir)/repmgr/repmgr_stat.c
	$(CC) $(CFLAGS) $?
repmgr_stub.lo: $(srcdir)/repmgr/repmgr_stub.c
	$(CC) $(CFLAGS) $?
repmgr_util.lo: $(srcdir)/repmgr/repmgr_util.c
	$(CC) $(CFLAGS) $?
rijndael-alg-fst.lo: $(srcdir)/crypto/rijndael/rijndael-alg-fst.c
	$(CC) $(CFLAGS) $?
rijndael-api-fst.lo: $(srcdir)/crypto/rijndael/rijndael-api-fst.c
	$(CC) $(CFLAGS) $?
seq_stat.lo: $(srcdir)/sequence/seq_stat.c
	 $(CC) $(CFLAGS) $?
sequence.lo: $(srcdir)/sequence/sequence.c
	 $(CC) $(CFLAGS) $?
sha1.lo: $(srcdir)/hmac/sha1.c
	$(CC) $(CFLAGS) $?
stat_stub.lo: $(srcdir)/common/stat_stub.c
	 $(CC) $(CFLAGS) $?
txn.lo: $(srcdir)/txn/txn.c
	 $(CC) $(CFLAGS) $?
txn_auto.lo: $(srcdir)/txn/txn_auto.c
	 $(CC) $(CFLAGS) $?
txn_autop.lo: $(srcdir)/txn/txn_autop.c
	 $(CC) $(CFLAGS) $?
txn_chkpt.lo: $(srcdir)/txn/txn_chkpt.c
	 $(CC) $(CFLAGS) $?
txn_failchk.lo: $(srcdir)/txn/txn_failchk.c
	 $(CC) $(CFLAGS) $?
txn_method.lo: $(srcdir)/txn/txn_method.c
	 $(CC) $(CFLAGS) $?
txn_rec.lo: $(srcdir)/txn/txn_rec.c
	 $(CC) $(CFLAGS) $?
txn_recover.lo: $(srcdir)/txn/txn_recover.c
	 $(CC) $(CFLAGS) $?
txn_region.lo: $(srcdir)/txn/txn_region.c
	 $(CC) $(CFLAGS) $?
txn_stat.lo: $(srcdir)/txn/txn_stat.c
	 $(CC) $(CFLAGS) $?
txn_util.lo: $(srcdir)/txn/txn_util.c
	 $(CC) $(CFLAGS) $?
util_arg.lo: $(srcdir)/common/util_arg.c
	 $(CC) $(CFLAGS) $?
util_cache.lo: $(srcdir)/common/util_cache.c
	 $(CC) $(CFLAGS) $?
util_log.lo: $(srcdir)/common/util_log.c
	 $(CC) $(CFLAGS) $?
util_sig.lo: $(srcdir)/common/util_sig.c
	 $(CC) $(CFLAGS) $?
uts4_cc.lo: $(srcdir)/mutex/uts4_cc.s
	$(AS) $(ASFLAGS) -o $@ $?
zerofill.lo: $(srcdir)/common/zerofill.c
	 $(CC) $(CFLAGS) $?

##################################################
# C++ API build rules.
##################################################
cxx_db.lo: $(srcdir)/cxx/cxx_db.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_dbc.lo: $(srcdir)/cxx/cxx_dbc.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_dbt.lo: $(srcdir)/cxx/cxx_dbt.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_env.lo: $(srcdir)/cxx/cxx_env.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_except.lo: $(srcdir)/cxx/cxx_except.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_lock.lo: $(srcdir)/cxx/cxx_lock.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_logc.lo: $(srcdir)/cxx/cxx_logc.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_mpool.lo: $(srcdir)/cxx/cxx_mpool.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_multi.lo: $(srcdir)/cxx/cxx_multi.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_seq.lo: $(srcdir)/cxx/cxx_seq.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_txn.lo: $(srcdir)/cxx/cxx_txn.cpp
	$(CXX) $(CXXFLAGS) $?

##################################################
# Java API build rules.
##################################################
db_java_wrap.lo: $(srcdir)/libdb_java/db_java_wrap.c
	$(CC) $(CFLAGS) $(SWIGCFLAGS) $?

##################################################
# STL API build rules.
##################################################
dbstl_container.lo: $(srcdir)/stl/dbstl_container.cpp
	$(CXX) $(STLFLAGS) $?
dbstl_resource_manager.lo: $(srcdir)/stl/dbstl_resource_manager.cpp
	$(CXX) $(STLFLAGS) $?

##################################################
# Tcl API build rules.
##################################################
tcl_compat.lo: $(srcdir)/tcl/tcl_compat.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_db.lo: $(srcdir)/tcl/tcl_db.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_db_pkg.lo: $(srcdir)/tcl/tcl_db_pkg.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_dbcursor.lo: $(srcdir)/tcl/tcl_dbcursor.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_env.lo: $(srcdir)/tcl/tcl_env.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_internal.lo: $(srcdir)/tcl/tcl_internal.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_lock.lo: $(srcdir)/tcl/tcl_lock.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_log.lo: $(srcdir)/tcl/tcl_log.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_mp.lo: $(srcdir)/tcl/tcl_mp.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_mutex.lo: $(srcdir)/tcl/tcl_mutex.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_rep.lo: $(srcdir)/tcl/tcl_rep.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_seq.lo: $(srcdir)/tcl/tcl_seq.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_txn.lo: $(srcdir)/tcl/tcl_txn.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?
tcl_util.lo: $(srcdir)/tcl/tcl_util.c
	$(CC) $(CFLAGS) $(TCL_INCLUDE_SPEC) $?

##################################################
# RPC build rules.
##################################################
# RPC client files
client.lo: $(srcdir)/rpc_client/client.c
	$(CC) $(CFLAGS) $?
db_server_clnt.lo: db_server_clnt.c
	$(CC) $(CFLAGS) $?
gen_client.lo: $(srcdir)/rpc_client/gen_client.c
	$(CC) $(CFLAGS) $?
gen_client_ret.lo: $(srcdir)/rpc_client/gen_client_ret.c
	$(CC) $(CFLAGS) $?

# RPC server files
db_server_proc.lo: $(srcdir)/rpc_server/c/db_server_proc.c
	$(CC) $(CFLAGS) $?
db_server_svc.lo: db_server_svc.c
	$(CC) $(CFLAGS) $?
db_server_util.lo: $(srcdir)/rpc_server/c/db_server_util.c
	$(CC) $(CFLAGS) $?
db_server_xdr.lo: db_server_xdr.c
	$(CC) $(CFLAGS) $?
gen_db_server.lo: gen_db_server.c
	$(CC) $(CFLAGS) $?
db_server_cxxproc.lo: $(srcdir)/rpc_server/cxx/db_server_cxxproc.cpp
	$(CXX) $(CXXFLAGS) $?
db_server_cxxutil.lo: $(srcdir)/rpc_server/cxx/db_server_cxxutil.cpp
	$(CXX) $(CXXFLAGS) $?

##################################################
# Utility build rules.
##################################################
db_archive.lo: $(srcdir)/db_archive/db_archive.c
	$(CC) $(CFLAGS) $?
db_checkpoint.lo: $(srcdir)/db_checkpoint/db_checkpoint.c
	$(CC) $(CFLAGS) $?
db_deadlock.lo: $(srcdir)/db_deadlock/db_deadlock.c
	$(CC) $(CFLAGS) $?
db_dump.lo: $(srcdir)/db_dump/db_dump.c
	$(CC) $(CFLAGS) $?
db_dump185.lo: $(srcdir)/db_dump185/db_dump185.c
	$(CC) $(DB185INC) $?
db_hotbackup.lo: $(srcdir)/db_hotbackup/db_hotbackup.c
	$(CC) $(CFLAGS) $?
db_load.lo: $(srcdir)/db_load/db_load.c
	$(CC) $(CFLAGS) $?
db_printlog.lo: $(srcdir)/db_printlog/db_printlog.c
	$(CC) $(CFLAGS) $?
db_recover.lo: $(srcdir)/db_recover/db_recover.c
	$(CC) $(CFLAGS) $?
db_stat.lo: $(srcdir)/db_stat/db_stat.c
	$(CC) $(CFLAGS) $?
db_upgrade.lo: $(srcdir)/db_upgrade/db_upgrade.c
	$(CC) $(CFLAGS) $?
db_verify.lo: $(srcdir)/db_verify/db_verify.c
	$(CC) $(CFLAGS) $?

db_sql.lo: $(srcdir)/db_sql/db_sql.c
	$(CC) $(CFLAGS) $?
preparser.lo: $(srcdir)/db_sql/preparser.c
	$(CC) $(CFLAGS) $?
parsefuncs.lo: $(srcdir)/db_sql/parsefuncs.c
	$(CC) $(CFLAGS) $?
tokenize.lo: $(srcdir)/db_sql/tokenize.c
	$(CC) $(CFLAGS) $?
buildpt.lo: $(srcdir)/db_sql/buildpt.c
	$(CC) $(CFLAGS) $?
utils.lo: $(srcdir)/db_sql/utils.c
	$(CC) $(CFLAGS) $?
generate.lo: $(srcdir)/db_sql/generate.c
	$(CC) $(CFLAGS) $?
generate_test.lo: $(srcdir)/db_sql/generate_test.c
	$(CC) $(CFLAGS) $?
generate_verification.lo: $(srcdir)/db_sql/generate_verification.c
	$(CC) $(CFLAGS) $?
generation_utils.lo: $(srcdir)/db_sql/generation_utils.c
	$(CC) $(CFLAGS) $?
hint_comment.lo: $(srcdir)/db_sql/hint_comment.c
	$(CC) $(CFLAGS) $?
sqlprintf.lo: $(srcdir)/db_sql/sqlite/sqlprintf.c
	$(CC) $(CFLAGS) $?
parse.lo: $(srcdir)/db_sql/sqlite/parse.c
	$(CC) $(CFLAGS) $?

##################################################
# C library replacement files.
##################################################
atoi.lo: $(srcdir)/clib/atoi.c
	$(CC) $(CFLAGS) $?
atol.lo: $(srcdir)/clib/atol.c
	$(CC) $(CFLAGS) $?
getcwd.lo: $(srcdir)/clib/getcwd.c
	$(CC) $(CFLAGS) $?
getopt.lo: $(srcdir)/clib/getopt.c
	$(CC) $(CFLAGS) $?
isalpha.lo: $(srcdir)/clib/isalpha.c
	$(CC) $(CFLAGS) $?
isdigit.lo: $(srcdir)/clib/isdigit.c
	$(CC) $(CFLAGS) $?
isprint.lo: $(srcdir)/clib/isprint.c
	$(CC) $(CFLAGS) $?
isspace.lo: $(srcdir)/clib/isspace.c
	$(CC) $(CFLAGS) $?
memcmp.lo: $(srcdir)/clib/memcmp.c
	$(CC) $(CFLAGS) $?
memcpy.lo: $(srcdir)/clib/memmove.c
	$(CC) -DMEMCOPY $(CFLAGS) $? -o $@
memmove.lo: $(srcdir)/clib/memmove.c
	$(CC) -DMEMMOVE $(CFLAGS) $?
printf.lo: $(srcdir)/clib/printf.c
	$(CC) $(CFLAGS) $?
qsort.lo: $(srcdir)/clib/qsort.c
	$(CC) $(CFLAGS) $?
raise.lo: $(srcdir)/clib/raise.c
	$(CC) $(CFLAGS) $?
rand.lo: $(srcdir)/clib/rand.c
	$(CC) $(CFLAGS) $?
strcasecmp.lo: $(srcdir)/clib/strcasecmp.c
	$(CC) $(CFLAGS) $?
strdup.lo: $(srcdir)/clib/strdup.c
	$(CC) $(CFLAGS) $?
snprintf.lo: $(srcdir)/clib/snprintf.c
	$(CC) $(CFLAGS) $?
strcat.lo: $(srcdir)/clib/strcat.c
	$(CC) $(CFLAGS) $?
strchr.lo: $(srcdir)/clib/strchr.c
	$(CC) $(CFLAGS) $?
strerror.lo: $(srcdir)/clib/strerror.c
	$(CC) $(CFLAGS) $?
strncat.lo: $(srcdir)/clib/strncat.c
	$(CC) $(CFLAGS) $?
strncmp.lo: $(srcdir)/clib/strncmp.c
	$(CC) $(CFLAGS) $?
strrchr.lo: $(srcdir)/clib/strrchr.c
	$(CC) $(CFLAGS) $?
strsep.lo: $(srcdir)/clib/strsep.c
	$(CC) $(CFLAGS) $?
strtol.lo: $(srcdir)/clib/strtol.c
	$(CC) $(CFLAGS) $?
strtoul.lo: $(srcdir)/clib/strtoul.c
	$(CC) $(CFLAGS) $?
time.lo: $(srcdir)/clib/time.c
	$(CC) $(CFLAGS) $?
