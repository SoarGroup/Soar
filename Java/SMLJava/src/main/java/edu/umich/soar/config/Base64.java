package edu.umich.soar.config;

import java.io.ByteArrayOutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

/** Base64 encoding and decoding functions. **/
public class Base64 {
	static char encodeLut[];
	static int decodeLut[];

	static {
		String cs = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

		assert (cs.length() == 64);

		encodeLut = new char[cs.length()];
		decodeLut = new int[256];

		for (int i = 0; i < cs.length(); i++) {
			encodeLut[i] = cs.charAt(i);
			decodeLut[(int) cs.charAt(i)] = i;
		}
	}

	public static String[] encode(byte in[]) {
		StringBuilder sb = new StringBuilder();
		List<String> lines = new ArrayList<String>();

		for (int idx = 0; idx < in.length; idx += 3) {
			int v0 = in[idx] & 0xff;
			int v1 = idx + 1 < in.length ? in[idx + 1] & 0xff : 0;
			int v2 = idx + 2 < in.length ? in[idx + 2] & 0xff : 0;
			int v = (v0 << 16) | (v1 << 8) | v2;

			int a = (v >> 18) & 63;
			int b = (v >> 12) & 63;
			int c = (v >> 6) & 63;
			int d = v & 63;

			sb.append(encodeLut[a]);
			sb.append(encodeLut[b]);
			sb.append(encodeLut[c]);
			sb.append(encodeLut[d]);

			if (idx + 3 >= in.length || sb.length() == 72) {
				if (idx + 2 >= in.length)
					sb.setCharAt(sb.length() - 1, '=');
				if (idx + 1 >= in.length)
					sb.setCharAt(sb.length() - 2, '=');

				lines.add(sb.toString());
				sb = new StringBuilder();
			}
		}

		return lines.toArray(new String[lines.size()]);
	}

	public static byte[] decode(String lines[]) {
		ByteArrayOutputStream outs = new ByteArrayOutputStream(
				54 * lines.length);

		for (int lineIdx = 0; lineIdx < lines.length; lineIdx++) {
			String line = lines[lineIdx];

			for (int idx = 0; idx < line.length(); idx += 4) {
				int a = decodeLut[(int) line.charAt(idx)];
				int b = decodeLut[(int) line.charAt(idx + 1)];
				int c = decodeLut[(int) line.charAt(idx + 2)];
				int d = decodeLut[(int) line.charAt(idx + 3)];

				int v = (a << 18) | (b << 12) | (c << 6) | d;

				int v0 = (v >> 16) & 0xff;
				int v1 = (v >> 8) & 0xff;
				int v2 = v & 0xff;

				outs.write(v0);
				if (line.charAt(idx + 2) != '=')
					outs.write(v1);
				if (line.charAt(idx + 3) != '=')
					outs.write(v2);
			}
		}

		return outs.toByteArray();
	}

	public static void main(String args[]) {
		Random r = new Random();

		// self test.

		for (int iter = 0; iter < 10000; iter++) {
			int len = r.nextInt(500);
			byte in[] = new byte[len];
			for (int i = 0; i < in.length; i++)
				in[i] = (byte) r.nextInt(256);

			String enc[] = encode(in);

			byte out[] = decode(enc);

//			if (false) {
//				for (int i = 0; i < enc.length; i++)
//					System.out.println(enc[i]);
//				System.out.println("");
//			}

			System.out.printf("%4d %4d\n", in.length, out.length);

			assert (in.length == out.length);
			if (true) {
				for (int i = 0; i < in.length; i++) {
					// System.out.printf("%2x %2x\n", in[i], out[i]);
					assert (in[i] == out[i]);
				}
			}

		}

		System.out.println("done");
	}
}
