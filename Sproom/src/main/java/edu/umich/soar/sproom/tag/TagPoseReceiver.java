package edu.umich.soar.sproom.tag;

import java.awt.BorderLayout;
import java.io.IOException;

import javax.swing.JFrame;

import april.lcmtypes.tag_pose_t;
import april.vis.VisCanvas;
import april.vis.VisChain;
import april.vis.VisRobot;
import april.vis.VisWorld;

import lcm.lcm.LCM;
import lcm.lcm.LCMDataInputStream;
import lcm.lcm.LCMSubscriber;

public class TagPoseReceiver implements LCMSubscriber
{

	LCM          lcm;
	JFrame       jf;
	VisWorld vw_map = new VisWorld();
    VisCanvas vc_map = new VisCanvas(vw_map);
	
	public TagPoseReceiver()
	{
		try{lcm=new LCM();}catch(IOException e){}
		jf = new JFrame(this.toString());
    	jf.setLayout(new BorderLayout());
    	jf.add(vc_map, BorderLayout.CENTER);

    	jf.setSize(1350,800);
    	jf.setVisible(true);
		lcm.subscribe("TAG_POSE", this);
	}
    public void messageReceived(LCM lcm, String channel, LCMDataInputStream ins)
    {
    	tag_pose_t tag=new tag_pose_t();
    	try {	
    		tag = new tag_pose_t(ins);
		} catch (IOException e) {}
		if(tag.ntags<=0)return;
		VisWorld.Buffer vb = vw_map.getBuffer("pose");
		for(int i=0;i<tag.ntags;i++)
		{
			System.out.println(i+" "+tag.id[i]+" "+tag.poses[i][0][3]+" "+tag.poses[i][1][3]);
			vb.addBuffered(new VisChain(tag.poses[i],new VisRobot()));
		}
		vb.switchBuffer();
    }
	/**
	 * @param args
	 */
	public static void main(String[] args) 
	{
		// TODO Auto-generated method stub
		new TagPoseReceiver();
	}

}
