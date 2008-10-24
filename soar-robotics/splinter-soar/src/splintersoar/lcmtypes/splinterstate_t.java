package splintersoar.lcmtypes;

import java.io.*;
import java.util.*;

public class splinterstate_t implements lcm.lcm.LCMEncodable {
	public long utime;
	public double baselinemeters;
	public double tickmeters;
	public double widthmeters;
	public double heightmeters;
	public int leftodom;
	public int rightodom;
	public lcmtypes.pose_t pose;

	public splinterstate_t() {
	}

	public static final long LCM_FINGERPRINT;
	public static final long LCM_FINGERPRINT_BASE = 0x320c885e82ea29f5L;

	static {
		LCM_FINGERPRINT = _hashRecursive(new ArrayList<Class>());
	}

	public static long _hashRecursive(ArrayList<Class> classes) {
		if (classes.contains(splintersoar.lcmtypes.splinterstate_t.class))
			return 0L;

		classes.add(splintersoar.lcmtypes.splinterstate_t.class);
		long hash = LCM_FINGERPRINT_BASE + lcmtypes.pose_t._hashRecursive(classes);
		classes.remove(classes.size() - 1);
		return (hash << 1) + ((hash >> 63) & 1);
	}

	public void encode(DataOutputStream outs) throws IOException {
		outs.writeLong(LCM_FINGERPRINT);
		_encodeRecursive(outs);
	}

	public void _encodeRecursive(DataOutputStream outs) throws IOException {
		byte[] __strbuf = null;
		outs.writeLong(this.utime);

		outs.writeDouble(this.baselinemeters);

		outs.writeDouble(this.tickmeters);

		outs.writeDouble(this.widthmeters);

		outs.writeDouble(this.heightmeters);

		outs.writeInt(this.leftodom);

		outs.writeInt(this.rightodom);

		this.pose._encodeRecursive(outs);

	}

	public splinterstate_t(DataInputStream ins) throws IOException {
		if (ins.readLong() != LCM_FINGERPRINT)
			throw new IOException("LCM Decode error: bad fingerprint");

		_decodeRecursive(ins);
	}

	public static splintersoar.lcmtypes.splinterstate_t _decodeRecursiveFactory(DataInputStream ins) throws IOException {
		splintersoar.lcmtypes.splinterstate_t o = new splintersoar.lcmtypes.splinterstate_t();
		o._decodeRecursive(ins);
		return o;
	}

	public void _decodeRecursive(DataInputStream ins) throws IOException {
		byte[] __strbuf = null;
		this.utime = ins.readLong();

		this.baselinemeters = ins.readDouble();

		this.tickmeters = ins.readDouble();

		this.widthmeters = ins.readDouble();

		this.heightmeters = ins.readDouble();

		this.leftodom = ins.readInt();

		this.rightodom = ins.readInt();

		this.pose = lcmtypes.pose_t._decodeRecursiveFactory(ins);

	}

	public splintersoar.lcmtypes.splinterstate_t copy() {
		splintersoar.lcmtypes.splinterstate_t outobj = new splintersoar.lcmtypes.splinterstate_t();
		outobj.utime = this.utime;

		outobj.baselinemeters = this.baselinemeters;

		outobj.tickmeters = this.tickmeters;

		outobj.widthmeters = this.widthmeters;

		outobj.heightmeters = this.heightmeters;

		outobj.leftodom = this.leftodom;

		outobj.rightodom = this.rightodom;

		outobj.pose = this.pose.copy();

		return outobj;
	}

}
