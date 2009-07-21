package edu.umich.soar.gridmap2d;

public class Arrays {
	public static int[] copyOf(int[] original, int newLength) {
		int[] a = new int[newLength];
		for (int i = 0; i < newLength; ++i) {
			if (i >= original.length)
				break;
			a[i] = original[i];
		}
		return a;
	}
	
	public static double[] copyOf(double[] original, int newLength) {
		double[] a = new double[newLength];
		for (int i = 0; i < newLength; ++i) {
			if (i >= original.length)
				break;
			a[i] = original[i];
		}
		return a;
	}
	
	private Arrays() {
		assert false;
	}
}
