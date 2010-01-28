package edu.umich.soar.sproom;

import java.util.InputMismatchException;
import java.util.NoSuchElementException;
import java.util.Scanner;

public class OdometryPoint {
	private final int left;
	private final int right;
	
	public static OdometryPoint newInstance(int left, int right) {
		return new OdometryPoint(left, right);
	}
	
	public static OdometryPoint valueOf(String value) {
		Scanner s = new Scanner(value).useDelimiter(",");
		try {
			return new OdometryPoint(s.nextInt(), s.nextInt());
		} catch (InputMismatchException e) {
		} catch (NoSuchElementException e) {
		} catch (IllegalStateException e) {
		}
		return null;
	}
	
	private OdometryPoint(int left, int right) {
		this.left = left;
		this.right = right;
	}
	
	public int getLeft() {
		return left;
	}
	
	public int getRight() {
		return right;
	}
	
	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append(left);
		sb.append(",");
		sb.append(right);
		return sb.toString();
	}
}
