package splintersoar.pf;

public class Particle implements Comparable< Particle >
{
	
	double [] pos = new double[2];
	double theta;
	double probability;
	double cumulative;
	
	@Override
	public int compareTo( Particle other ) 
	{
		return Double.compare( this.cumulative, other.cumulative );
	}
	
	@Override
	public String toString()
	{
		StringBuilder sb = new StringBuilder();
		sb.append( "(" );
		sb.append( pos[0] );
		sb.append( "," );
		sb.append( pos[1] );
		sb.append( "," );
		sb.append( theta );
		sb.append( "," );
		sb.append( probability );
		sb.append( ")" );
		
		return sb.toString();
	}
	
}
