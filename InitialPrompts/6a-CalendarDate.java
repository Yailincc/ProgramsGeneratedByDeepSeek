import java.io.Serializable;
import java.util.Date;

// Serializable class representing a calendar event
public class CalendarEvent implements Serializable {
    private static final long serialVersionUID = 1L;
    private Date date;
    private String eventDescription;

    public CalendarEvent(Date date, String eventDescription) {
        this.date = date;
        this.eventDescription = eventDescription;
    }

    public Date getDate() {
        return date;
    }

    public String getEventDescription() {
        return eventDescription;
    }

    @Override
    public String toString() {
        return "CalendarEvent [date=" + date + ", eventDescription=" + eventDescription + "]";
    }
}
