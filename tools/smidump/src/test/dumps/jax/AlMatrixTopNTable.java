/*
 * This Java file has been generated by smidump 0.3.0. Do not edit!
 * It is intended to be used within a Java AgentX sub-agent environment.
 *
 * $Id: AlMatrixTopNTable.java,v 1.10 2002/03/05 14:27:09 strauss Exp $
 */

/**
    This class represents a Java AgentX (JAX) implementation of
    the table alMatrixTopNTable defined in RMON2-MIB.

    @version 1
    @author  smidump 0.3.0
    @see     AgentXTable
 */

import java.util.Vector;

import jax.AgentXOID;
import jax.AgentXVarBind;
import jax.AgentXResponsePDU;
import jax.AgentXSetPhase;
import jax.AgentXTable;
import jax.AgentXEntry;

public class AlMatrixTopNTable extends AgentXTable
{

    // entry OID
    private final static long[] OID = {1, 3, 6, 1, 2, 1, 16, 17, 4, 1};

    // constructors
    public AlMatrixTopNTable()
    {
        oid = new AgentXOID(OID);

        // register implemented columns
        columns.addElement(new Long(2));
        columns.addElement(new Long(3));
        columns.addElement(new Long(4));
        columns.addElement(new Long(5));
        columns.addElement(new Long(6));
        columns.addElement(new Long(7));
        columns.addElement(new Long(8));
        columns.addElement(new Long(9));
    }

    public AlMatrixTopNTable(boolean shared)
    {
        super(shared);

        oid = new AgentXOID(OID);

        // register implemented columns
        columns.addElement(new Long(2));
        columns.addElement(new Long(3));
        columns.addElement(new Long(4));
        columns.addElement(new Long(5));
        columns.addElement(new Long(6));
        columns.addElement(new Long(7));
        columns.addElement(new Long(8));
        columns.addElement(new Long(9));
    }

    public AgentXVarBind getVarBind(AgentXEntry entry, long column)
    {
        AgentXOID oid = new AgentXOID(getOID(), column, entry.getInstance());

        switch ((int)column) {
        case 2: // alMatrixTopNProtocolDirLocalIndex
        {
            int value = ((AlMatrixTopNEntry)entry).get_alMatrixTopNProtocolDirLocalIndex();
            return new AgentXVarBind(oid, AgentXVarBind.INTEGER, value);
        }
        case 3: // alMatrixTopNSourceAddress
        {
            byte[] value = ((AlMatrixTopNEntry)entry).get_alMatrixTopNSourceAddress();
            return new AgentXVarBind(oid, AgentXVarBind.OCTETSTRING, value);
        }
        case 4: // alMatrixTopNDestAddress
        {
            byte[] value = ((AlMatrixTopNEntry)entry).get_alMatrixTopNDestAddress();
            return new AgentXVarBind(oid, AgentXVarBind.OCTETSTRING, value);
        }
        case 5: // alMatrixTopNAppProtocolDirLocalIndex
        {
            int value = ((AlMatrixTopNEntry)entry).get_alMatrixTopNAppProtocolDirLocalIndex();
            return new AgentXVarBind(oid, AgentXVarBind.INTEGER, value);
        }
        case 6: // alMatrixTopNPktRate
        {
            long value = ((AlMatrixTopNEntry)entry).get_alMatrixTopNPktRate();
            return new AgentXVarBind(oid, AgentXVarBind.GAUGE32, value);
        }
        case 7: // alMatrixTopNReversePktRate
        {
            long value = ((AlMatrixTopNEntry)entry).get_alMatrixTopNReversePktRate();
            return new AgentXVarBind(oid, AgentXVarBind.GAUGE32, value);
        }
        case 8: // alMatrixTopNOctetRate
        {
            long value = ((AlMatrixTopNEntry)entry).get_alMatrixTopNOctetRate();
            return new AgentXVarBind(oid, AgentXVarBind.GAUGE32, value);
        }
        case 9: // alMatrixTopNReverseOctetRate
        {
            long value = ((AlMatrixTopNEntry)entry).get_alMatrixTopNReverseOctetRate();
            return new AgentXVarBind(oid, AgentXVarBind.GAUGE32, value);
        }
        }

        return null;
    }

    public int setEntry(AgentXSetPhase phase,
                        AgentXEntry entry,
                        long column,
                        AgentXVarBind vb)
    {

        switch ((int)column) {
        }

        return AgentXResponsePDU.NOT_WRITABLE;
    }

}

